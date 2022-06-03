//#include <stdafx.h>

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFSIZE     100
#define PORT 9370

#define BLACK 99
#define WHITE 88
#define EMPTY 0

#define MAX_X 15
#define MAX_Y 15

void ErrorHandling(char* message);

int Matrix[MAX_X][MAX_Y] = { {0,} };

int main(int argc, char** argv)
{

	WSADATA wsaData;
	SOCKET hServSock;
	SOCKADDR_IN servAddr;

	int arrIndex;
	int clntLen;
	SOCKET hClntSock;
	SOCKADDR_IN clntAddr;
	
	fd_set reads, temps;
	
	char message[BUFSIZE];
	int strLen;
	TIMEVAL timeout;
	
	char send_message[BUFSIZE];
	char* result;
	char temp_res[5];
	char error_msg[BUFSIZE];

	char status;
	char whose;
	char turn;
	int posX, posY;
	int i = 0;

	int black_count = 0;
	int white_count = 0;

	char winner;

	char* str = NULL;

	// 바둑판 초기화
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			Matrix[i][j] == EMPTY;
		}
	}
	
	
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	if (hServSock == INVALID_SOCKET) {
		ErrorHandling("socket() error");

	}
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(PORT);


	if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
		ErrorHandling("bind() error");
		
	}

	if (listen(hServSock, 5) == SOCKET_ERROR) {
		ErrorHandling("listen() error");
		
	}
	
    FD_ZERO(&reads);
	FD_SET(hServSock, &reads);
	
	while (1) {
		temps = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		
		if (select(0, &temps, 0, 0, &timeout) == SOCKET_ERROR) {
			ErrorHandling("select() error");
		}

		for (arrIndex = 0; arrIndex < reads.fd_count; arrIndex++) {
			if (FD_ISSET(reads.fd_array[arrIndex], &temps)) {
				if (reads.fd_array[arrIndex] == hServSock) {
					clntLen = sizeof(clntAddr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntLen);
					FD_SET(hClntSock, &reads);
					printf("client connected : socket handle %d\n", hClntSock);
				
				}
				else {
					strLen = recv(reads.fd_array[arrIndex], message, BUFSIZE - 1, 0);
					if (strLen == 0) {
						closesocket(temps.fd_array[arrIndex]);
						printf("client disconnected : socket handle %d\n", reads.fd_array[arrIndex]);

						FD_CLR(reads.fd_array[arrIndex], &reads);
						
					}
					else {
						message[strLen] = '\0';
						printf("client %d : %s\n", reads.fd_array[arrIndex], message);


						// 승패 여부 결정
						// 순서도 결정
						/*
							<send할때 보내야 할 것>
							1. 상태 (승자 여부)
							2. 순서 (누구의 차례인지)
							3. 받아온 X좌표
							4. 받아온 Y좌표

							이것들을 ','로 구분하여(즉, 문자열 합쳐야 함) 상대방에게 send하면 됨.
						*/
						sscanf(message, "%c,%c,%d,%d", &status,&whose,&posX, &posY);
						
						printf("%c\t%c\t%d\t%d\n", status, whose, posX, posY);

						if (Matrix[posY][posX] == BLACK || Matrix[posY][posX] == WHITE) {
							sprintf(error_msg, "%s", "Exist Stone!");
							send(reads.fd_array[arrIndex], error_msg, sizeof(error_msg), 0);
						}
						else {
							if (whose == 'A' || whose == 'a') {
								Matrix[posY][posX] = BLACK;
							}
							else if (whose == 'B' || whose == 'b') {
								Matrix[posY][posX] = WHITE;
							}


							for (int i = 0; i < MAX_X; i++) {
								for (int j = 0; j < MAX_Y; j++) {
									if (Matrix[i][j] == EMPTY) {
										printf("+\t");
									}
									if (Matrix[i][j] == WHITE) {
										printf("○\t");
									}
									if (Matrix[i][j] == BLACK) {
										printf("●\t");
									}
								}
								printf("\n");
							}


							// 가로(Left to Right)로 검사
							printf("가로(RtoL) 검사\n");
							for (int i = 0; i < MAX_X; i++) {
								for (int j = 0; j < MAX_Y; j++) {
									if (Matrix[i][j] == WHITE) {
										++white_count;
										black_count = 0;
									}

									else if (Matrix[i][j] == BLACK) {
										++black_count;
										white_count = 0;
									}
								}

								printf("Line Number : %d\tWhite Count : %d\n", i, white_count);
								printf("Line Number : %d\tBlack Count : %d\n", i, black_count);
								printf("\n");

								if (black_count == 5) {
									winner = 'A';
									break;
								}
								else if (white_count == 5) {
									winner = 'B';
									break;
								}
								else {
									winner = 'N';
								}

								black_count = 0;
								white_count = 0;


							}

							// 가로(Right to Left)검사
							printf("가로(LtoR) 검사\n");
							for (int i = MAX_X; i > 0; i--) {
								for (int j = MAX_Y; j > 0; j--) {
									if (Matrix[i][j] == WHITE) {
										++white_count;
										black_count = 0;
									}

									else if (Matrix[i][j] == BLACK) {
										++black_count;
										white_count = 0;
									}
								}

								printf("Line Number : %d\tWhite Count : %d\n", i, white_count);
								printf("Line Number : %d\tBlack Count : %d\n", i, black_count);
								printf("\n");

								if (black_count == 5) {
									winner = 'A';
									break;
								}
								else if (white_count == 5) {
									winner = 'B';
									break;
								}
								else {
									winner = 'N';
								}

								black_count = 0;
								white_count = 0;

							}

							printf("\n");
							printf("세로 검사\n\n");

							// 세로(Top to Bottom)로 검사
							for (int i = 0; i < MAX_X; i++) {
								for (int j = 0; j < MAX_Y; j++) {
									if (Matrix[j][i] == WHITE) {
										++white_count;
										black_count = 0;
									}
									else if (Matrix[j][i] == BLACK) {
										++black_count;
										white_count = 0;
									}
								}

								printf("Line Number : %d\tWhite Count : %d\n", i ,white_count);
								printf("Line Number : %d\tBlack Count : %d\n", i ,black_count);
								printf("\n");

								if (black_count == 5) {
									winner = 'A';
									break;
								}
								else if (white_count == 5) {
									winner = 'B';
									break;
								}
								else {
									winner = 'N';
								}

								black_count = 0;
								white_count = 0;								
							}

							// 세로(Bottom to Top)로 검사
							for (int i = MAX_X; i > 0; i--) {
								for (int j = MAX_Y; j > 0; j--) {
									if (Matrix[j][i] == WHITE) {
										++white_count;
										black_count = 0;
									}
									else if (Matrix[j][i] == BLACK) {
										++black_count;
										white_count = 0;
									}
								}

								printf("Line Number : %d\tWhite Count : %d\n", i, white_count);
								printf("Line Number : %d\tBlack Count : %d\n", i, black_count);
								printf("\n");

								if (black_count == 5) {
									winner = 'A';
									break;
								}
								else if (white_count == 5) {
									winner = 'B';
									break;
								}
								else {
									winner = 'N';
								}

								black_count = 0;
								white_count = 0;
							}


						}


						if (whose == 'A' || whose == 'a') {
							sprintf(send_message, "%c,%c,%d,%d", winner, 'B', posX, posY);
						}
						else if (whose == 'B' || whose == 'b') {
							sprintf(send_message, "%c,%c,%d,%d", winner, 'A', posX, posY);
						}

						// 상대방 클라이언트에게 정보를 전송하는 부분
						for (int idx = 0; idx < reads.fd_count; idx++) {
							if (idx == arrIndex) { continue; }
							send(reads.fd_array[idx], send_message, sizeof(send_message), 0);
						}
					
					}
				}
			}
		}
	}
	

	WSACleanup();	
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
