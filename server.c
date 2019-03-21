#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 5
#define BUFLEN 256

void error(char *msg)
{
	perror(msg);
	exit(1);
}
typedef struct {

	char nume[12]; //numele clientului
	char prenume[12]; //prenumele clientului
	int numar_card; //numarul cardului asociat
	int pin; //pinul cardului
	char parola[8]; //parola secreta
	double sold; //suma de bani existenta in cont
	int status; //statusul contului; 1, daca este logat cineva cu contul acesta. 0, altfel
	int incercari; //numarul de incercari de logare
	int numberClient;

} client;

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno, clilen;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, j, k;

	fd_set read_fds; //multimea de citire folosita in select()
	fd_set tmp_fds;  //multime folosita temporar
	int fdmax;		 //valoare maxima file descriptor din multimea read_fds

	if (argc < 3) {
		fprintf(stderr, "Usage : %s port\n", argv[0]);
		exit(1);
	}

	FILE* file;
	file = fopen(argv[2], "r");

	if (file == NULL) {
		printf("-10 : Eroare la apelul fisierului\n");
	}

	//Numarul de clienti existenti in fisierul dat ca parametru
	int numberClients = 0;
	fscanf(file, "%d", &numberClients);

	//Vector de structuri de tip client ce retine toate informatiile din
	//fisierul primit ca parametru
	client arrayClients[numberClients];
	for (int i = 0; i < numberClients; i++) {
		fscanf(file, "%s", arrayClients[i].nume);
		fscanf(file, "%s", arrayClients[i].prenume);
		fscanf(file, "%d", &arrayClients[i].numar_card);
		fscanf(file, "%d", &arrayClients[i].pin);
		fscanf(file, "%s", arrayClients[i].parola);
		fscanf(file, "%lf", &arrayClients[i].sold);
		arrayClients[i].status = 0;
		arrayClients[i].incercari = 0;
	}

	fclose(file);
	//Functie pentru afisarea datelor din array. Pentru verificare
	/*for (int i = 0; i < numberClients; i++) {
	printf("Nume:%s Prenume:%s Numar Card:%d Pin:%d Parola: %s Sold %lf",
	arrayClients[i].nume, arrayClients[i].prenume, arrayClients[i].numar_card,
	arrayClients[i].pin, arrayClients[i].parola, arrayClients[i].sold);
	}*/

	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("-10 : Eroare la apel socket\n");
	}

	portno = atoi(argv[1]);

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // foloseste adresa IP a masinii
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
		printf("-10 : Eroare la apel bind\n");
	}

	listen(sockfd, MAX_CLIENTS);

	//adaugam noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);
	fdmax = sockfd;

	// main loop
	while (1) {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			printf("-10 : Eroare la apel select\n");
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				//Citire de la tastatura
				if (i == 0) {
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN, stdin); 
					//Daca primesc de la stdin mesajul de quit inchid serverul
					if (strncmp(buffer, "quit", 4) == 0) {
						for (j = 4; j <= fdmax; ++j) {
								send(j, buffer, BUFLEN, 0);
								FD_CLR(j, &read_fds);
						}
						close(sockfd);
						return 0;
					}
				}


				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()
					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						printf("-10 : Eroare la apel accept\n");
					}
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) {
							fdmax = newsockfd;
						}
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n ", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
				}

				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recvfrom(i, buffer, BUFLEN, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr)) <= 0)) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("-10 : Eroare la apel recv - inchidere socket\n");
						}
						else {
							printf("-10 : Eroare la apel recv\n");
						}
						close(i);
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care
					}

					else { //recv intoarce >0 => am primit un mesaj
						int command = 0; //Parametru folosit pentru a asigura faptul ca la introducerea
								//unei noi comenzi, neimplementate, nu se blocheaza serverul.
						//printf("Am primit de la clientul de pe socketul %d, mesajul: %s\n", i, buffer);
						//memset(buffer, 0, BUFLEN);

						if (strncmp(buffer, "login", 5) == 0) {
							command = 1;
							int nrCard = 0;
							int nrPin = 0;
							char * space;

							space = strtok(buffer, " "); //primul parametru login

							space = strtok(NULL, " "); //nr card
							nrCard = atoi(space);

							space = strtok(NULL, " \n"); //nrPin
							nrPin = atoi(space);

							memset(buffer, 0, BUFLEN);
							int found = 0;
							//verificam daca exista o conexiune deja deschisa pe socketul respectiv
							for (int j = 0; j < numberClients; j++) {
								if (arrayClients[j].numberClient == i) {
									found++;
								}
							}
							if (found == 1) {
								strcpy(buffer, "-2 Sesiune deja deschisa");
								send(i, buffer, BUFLEN, 0);
							}
							else {
								found = 0;
								for (int j = 0; j < numberClients; j++) {
									if (arrayClients[j].numar_card == nrCard) {

										if (arrayClients[j].status == 1) {
											printf("IBANK > -2 Sesiune deja deschisa\n");
											strcpy(buffer, "IBANK > -2 Sesiune deja deschisa");
											send(i, buffer, BUFLEN, 0);
											found = 1;
											break;
										}
										found = 1;
										if (arrayClients[j].status == 0) {
											if (arrayClients[j].incercari >= 3) {
												printf("IBANK > - 5 Card blocat\n");
												strcpy(buffer, "IBANK > - 5 Card blocat");
												send(i, buffer, BUFLEN, 0);
												break;
											}
											else {
												if (arrayClients[j].pin != nrPin || nrPin > 9999 || nrPin < 1000) {
													printf("IBANK > -3 Pin gresit\n");
													arrayClients[j].incercari++;
													strcpy(buffer, "IBANK > -3 Pin gresit");
													send(i, buffer, BUFLEN, 0);
													break;
												}
												else {
													printf("IBANK > Welcome %s %s\n", arrayClients[j].nume,
														arrayClients[j].prenume);
													arrayClients[j].status = 1;
													arrayClients[j].incercari = 0;
													arrayClients[j].numberClient = i;
													strcpy(buffer, "IBANK > Welcome ");
													strcat(buffer, arrayClients[j].nume);
													strcat(buffer, " ");
													strcat(buffer, arrayClients[j].prenume);
													send(i, buffer, BUFLEN, 0);
													break;
												}
											}

										}
										break;
									}
								}
								if (found == 0) {
									printf("IBANK > -4 Numar card inexistent");
									strcpy(buffer, "IBANK > -4 Numar card inexistent");
									send(i, buffer, BUFLEN, 0);
								}
								memset(buffer, 0, BUFLEN);
							}
						}

						if (strncmp(buffer, "logout", 6) == 0) {
							command = 1;
							memset(buffer, 0, BUFLEN);
							int found = 0;
							for (int j = 0; j < numberClients; j++) {
								if (arrayClients[j].incercari >= 3 && arrayClients[j].numberClient == i) {
									found = 1;
									printf("IBANK > -5 : Card blocat\n");
									strcpy(buffer, "IBANK > -5 : Card blocat");
									send(i, buffer, BUFLEN, 0);
									break;
								}
								if (arrayClients[j].numberClient == i && arrayClients[j].status == 1) {
									found = 1;
									arrayClients[j].status = 0;
									printf("IBANK > Clientul a fost deconectat\n");
									strcpy(buffer, "IBANK > Clientul a fost deconectat");
									send(i, buffer, BUFLEN, 0);
									break;
								}
							}
							if (found == 0) {
								printf("-1 : Clientul nu este autentificat\n");
								strcpy(buffer, "-1 : Clientul nu este autentificat");
								send(i, buffer, BUFLEN, 0);
							}
							memset(buffer, 0, BUFLEN);
						}

						if (strncmp(buffer, "listsold", 8) == 0) {
							command = 1;
							memset(buffer, 0, BUFLEN);
							//printf("Intrat caz listsold\n");
							int found = 0;

							for (int j = 0; j < numberClients; j++) {
								if (arrayClients[j].incercari >= 3 && arrayClients[j].numberClient == i) {
									found = 1;
									printf("IBANK > -5 : Card blocat\n");
									strcpy(buffer, "IBANK > -5 : Card blocat");
									send(i, buffer, BUFLEN, 0);
									break;
								}
								if (arrayClients[j].numberClient == i && arrayClients[j].status == 1) {
									found = 1;
									printf("IBANK > %lf\n", arrayClients[j].sold);
									strcpy(buffer, "IBANK > ");
									char stringSold[20];
									sprintf(stringSold, "%0.2f", arrayClients[j].sold);
									strcat(buffer, stringSold);
									send(i, buffer, BUFLEN, 0);
									break;
								}
							}
							if (found == 0) {
								printf("-1 : Clientul nu este autentificat\n");
								strcpy(buffer, "-1 : Clientul nu este autentificat");
								send(i, buffer, BUFLEN, 0);
							}
						}

						if (strncmp(buffer, "transfer", 8) == 0) {
							command = 1;
							int nrCard, suma;
							char *space;
							space = strtok(buffer, " "); //primul parametru transfer

							space = strtok(NULL, " "); //nr card
							nrCard = atoi(space);

							space = strtok(NULL, " \n"); //nrPin
							suma = atoi(space);

							memset(buffer, 0, BUFLEN);
							int found = -1;
							int foundNrCard = -1;
							for (int j = 0; j < numberClients; j++) {
								if (arrayClients[j].numar_card == nrCard) {
									foundNrCard = j;
									//printf("Gasit card dorit\n");
								}
								if (arrayClients[j].numberClient == i) {
									found = j;
									//printf("Gasit card curent\n");
								}
							}
							if (foundNrCard == -1 || found == -1) {
								printf("IBANK > -4 Numar card inexistent\n");
								strcpy(buffer, "IBANK > -4 Numar card inexistent");
								send(i, buffer, BUFLEN, 0);
							}

							else {
								if (arrayClients[found].sold < suma) {
									printf("IBANK > -8 Fonduri insuficiente\n");
									strcpy(buffer, "IBANK > -8 Fonduri insuficiente");
									send(i, buffer, BUFLEN, 0);
								}
								else {
									//printf("Fonduri suficiente!\n");
									memset(buffer, 0, BUFLEN);
									strcpy(buffer, "IBANK > Transfer ");
									char stringSold[20];
									sprintf(stringSold, "%d", suma);
									strcat(buffer, stringSold);
									strcat(buffer, " catre ");
									strcat(buffer, arrayClients[foundNrCard].nume);
									strcat(buffer, " ");
									strcat(buffer, arrayClients[foundNrCard].prenume);
									strcat(buffer, "? [y/n]");
									send(i, buffer, BUFLEN, 0);
									if (recv(i, buffer, BUFLEN, 0) > 0) {
										if (strncmp(buffer, "y", 1) == 0) {
											arrayClients[foundNrCard].sold += suma;
											arrayClients[found].sold -= suma;
											memset(buffer, 0, BUFLEN);
											printf("IBANK > Transfer realizat cu succes\n");
											strcpy(buffer, "IBANK > Transfer realizat cu succes");
											send(i, buffer, BUFLEN, 0);
										}
										else {
											printf("IBANK > -9 Operatie anulata\n");
											strcpy(buffer, "IBANK > -9 Operatie anulata");
											send(i, buffer, BUFLEN, 0);
										}
									}
								}
							}
							memset(buffer, 0, BUFLEN);
						}

						if (strncmp(buffer, "quit", 4) == 0) {
							command = 1;
							memset(buffer, 0, BUFLEN);
							int found = 0;
							for (int j = 0; j < numberClients; j++) {
								if (arrayClients[j].incercari >= 3 && arrayClients[j].numberClient == i) {
									found = 1;
									printf("IBANK > -5 : Card blocat\n");
									strcpy(buffer, "IBANK > -5 : Card blocat");
									send(i, buffer, BUFLEN, 0);
									break;
								}
								if (arrayClients[j].numberClient == i && arrayClients[j].status == 1) {
									found = 1;
									arrayClients[j].status = 0;
									printf("IBANK > Clientul a inchis conexiunea\n");
									FD_CLR(i, &read_fds);
									break;
								}
							}
							if (found == 0) {
								printf("-1 : Clientul nu este autentificat\n");
								FD_CLR(i, &read_fds);
							}
							memset(buffer, 0, BUFLEN);
						}

						if (command == 0) {
							printf("IBANK > - 10 : Eroare la apel functie\n");
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, "IBANK > - 10 : Eroare la apel functie");
							send(i, buffer, BUFLEN, 0);
						}
					}
				}
			}
		}
	}

	close(sockfd);

	return 0;
}
