#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define BUFLEN 256

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set read_fds;
    fd_set tmp_fds; //multime folosita temporar
	int state_login = 0;

    char buffer[BUFLEN];
    if (argc < 3)
    {
        fprintf(stderr, "Usage %s server_address server_port\n", argv[0]);
        exit(0);
    }

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("-10 : Eroare la apel socket");
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &serv_addr.sin_addr);

	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		error("-10 : Eroare la apel connect");
	}

    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    int fdmax = sockfd;
    int i;

	//fisierul de log
	char logFile[25];
	memset(logFile, 0, 25);
	strcpy(logFile, "client-");
	char pidClient[20];
	sprintf(pidClient, "%d", getpid());
	strcat(logFile, pidClient);
	strcat(logFile, ".log");

	FILE* f = fopen(logFile, "wt");
    while (1)
    {
        tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			error("-10 : Eroare la apel select");
		}

        for (i = 0; i <= sockfd; i++)
            if (FD_ISSET(i, &tmp_fds))
            {
                if (i == 0)
                {
                    //citesc de la tastatura
                    memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin);

					fwrite(buffer, strlen(buffer), 1, f);

                    //trimit mesaj la server
                    n = send(sockfd, buffer, strlen(buffer), 0);
                    if (n < 0)
                        error("ERROR writing to socket");

					if (strncmp(buffer, "quit", 4) == 0) {
						fclose(f);
						close(sockfd);
						return 0;
					}
                }
                else
                {
                    socklen_t slen;
                    int rc = recvfrom(i, buffer, BUFLEN, 0, (struct sockaddr *)&serv_addr, &slen);
                    if (rc < 0)
                    {
                        perror("Recv Error!\n");
                        exit(-1);
                    }
					else {
						printf("%s\n", buffer);
						if (strncmp(buffer, "quit", 4) == 0) {
							fwrite("IBANK > Clientul a inchis conexiunea\n", 38, 1, f);
							fclose(f);
							close(sockfd);
							return 0;
						}
						strcat(buffer, "\n");
						strcat(buffer, "\n");
						fwrite(buffer, strlen(buffer), 1, f);
					}
                }
            }
    }
	fclose(f);
    close(sockfd);
    return 0;
}
