#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>

#define PORT 9000
#define BACKLOG 10
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define RECV_BUF_SIZE 1024

static int server_fd = -1;
static volatile sig_atomic_t exit_requested = 0;

static void handle_signal(int signo)
{
    syslog(LOG_INFO, "Signal catch, exiting");
    exit_requested = 1;

    if (server_fd != -1) 
    {
        close(server_fd);
        server_fd = -1;
    }

    remove(DATA_FILE);
}


static void daemonize(void)
{
    pid_t pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() == -1)
        exit(EXIT_FAILURE);


    int fd = open("/dev/null", O_RDWR);

    if (fd != -1) 
    {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > 2)
            close(fd);
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int client_fd;
    int daemon_mode = 0;
    int opt;

    while ((opt = getopt(argc, argv, "d")) != -1)
    {
        if (opt == 'd')
        {
            syslog(LOG_INFO, "Entering daemon mode");
            daemon_mode = 1;
        }
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) 
    {
        syslog(LOG_ERR, "socket failed");
        return -1;
    }

    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) 
    {
        syslog(LOG_ERR, "bind failed");
        close(server_fd);
        return -1;
    }

    if (daemon_mode)
        daemonize();

    if (listen(server_fd, BACKLOG) != 0)
    {
        syslog(LOG_ERR, "listen failed");
        close(server_fd);
        return -1;
    }

    while (!exit_requested)
    {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1)
        {
            if (exit_requested)
                break;
            
            syslog(LOG_ERR, "accept failed");
            continue;
        }

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        syslog(LOG_INFO, "Successful connect from %s", client_ip);

        char recv_buf[RECV_BUF_SIZE];
        char *packet = NULL;
        size_t packet_size = 0;

        while (1)
        {
            ssize_t bytes = recv(client_fd, recv_buf, sizeof(recv_buf), 0);
            if (bytes <= 0)
                break;

            size_t offset = 0;
            char *newline;

            while ((newline = memchr(recv_buf + offset, '\n', bytes - offset)) != NULL)
            {
                size_t chunk_len = newline - (recv_buf + offset) + 1;
                char *tmp = realloc(packet, packet_size + chunk_len);
                if (!tmp) 
                {
                    syslog(LOG_ERR, "realloc failed");
                    free(packet);
                    packet = NULL;
                    packet_size = 0;
                    break;
                }
                
                packet = tmp;
                memcpy(packet + packet_size, recv_buf + offset, chunk_len);
                packet_size += chunk_len;

                FILE *fp = fopen(DATA_FILE, "a+");
                if (fp)
                {
                    fwrite(packet, 1, packet_size, fp);
                    fclose(fp);
                }

                fp = fopen(DATA_FILE, "r");
                if (fp)
                {
                    char file_buf[RECV_BUF_SIZE];
                    size_t r;
                    while ((r = fread(file_buf, 1, sizeof(file_buf), fp)) > 0)
                    {
                        send(client_fd, file_buf, r, 0);
                    }
                    fclose(fp);
                }

                free(packet);
                packet = NULL;
                packet_size = 0;
                offset += chunk_len;
            }

            if (offset < (size_t)bytes)
            {
                size_t remaining = bytes - offset;
                char *tmp = realloc(packet, packet_size + remaining);
                if (!tmp)
                {
                    syslog(LOG_ERR, "realloc failed");
                    free(packet);
                    packet = NULL;
                    packet_size = 0;
                    break;
                }

                packet = tmp;
                memcpy(packet + packet_size, recv_buf + offset, remaining);
                packet_size += remaining;
            }
        }

        free(packet);
        close(client_fd);
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }
    
    closelog();
    return 0;
}

