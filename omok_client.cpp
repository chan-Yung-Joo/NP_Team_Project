//#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>


#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>


struct Data {
    HANDLE mx;
    bool valid;
    char data[256];
};


#define BUFSIZE 128
#define IP_ADDRESS "127.0.0.1"
#define PORT_NUMBER 9370

#pragma comment(lib, "ws2_32.lib")
struct SocketParam {
    char address[128];
    int port;

    Data* sdata;
    Data* rdata;
};
DWORD WINAPI SocketThread(LPVOID lpParam)
{
    SocketParam* param = (SocketParam*)lpParam;
    SOCKET sock;
    SOCKADDR_IN serverAddress;

    int retval;
    char buf[BUFSIZE + 1];
    int length;
    WSADATA wsa;

    //¼ÒÄÏ ¶óÀÌºê·¯¸® ÃÊ±âÈ­
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        return -1;
    }

    //¼ÒÄÏ »ý¼º
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("socket() failed");
        return -1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(param->address);
    serverAddress.sin_port = htons(param->port);

    //¼­¹ö¿¡ ¿¬°á ¿äÃ»
    retval = connect(sock, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (retval == SOCKET_ERROR) {
        printf("connect() failed.");
        closesocket(sock);
        //¼ÒÄÏ ¶óÀÌºê·¯¸® ÇØÁ¦
        WSACleanup();
        return -1;
    }

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        TIMEVAL timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10 * 1000;
        if (select(0, &fds, 0, 0, &timeout) == SOCKET_ERROR) {
            break;
        }

        if (FD_ISSET(sock, &fds)) {
            char buf[256];
            int slen = recv(sock, buf, BUFSIZE - 1, 0);
            if (slen == 0) {
                printf("recv failed.");
                break;
            }
            else {
                buf[slen] = '\0';

                WaitForSingleObject(param->rdata->mx, INFINITE);
                strcpy_s(param->rdata->data, buf);
                param->rdata->valid = true;
                ReleaseMutex(param->rdata->mx);
            }
        }

        WaitForSingleObject(param->sdata->mx, INFINITE);
        if (param->sdata->valid) {
            retval = send(sock, param->sdata->data, strlen(param->sdata->data), 0);
            param->sdata->valid = false;
            if (retval == SOCKET_ERROR) {
                ReleaseMutex(param->sdata->mx);
                printf("send() failed");
                break;
            }
        }
        ReleaseMutex(param->sdata->mx);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

DWORD WINAPI InputThread(LPVOID lpParam)
{
    Data* sdata = (Data*)lpParam;

    char buf[256];
    while (true) {
        ZeroMemory(buf, sizeof(buf));
        printf("send to server : ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL) {
            break;
        }

        if (buf[0] == '\n') {
            continue;
        }

        WaitForSingleObject(sdata->mx, INFINITE);
        strcpy_s(sdata->data, buf);
        sdata->valid = true;
        ReleaseMutex(sdata->mx);
    }
    return 0;
}


int main(int argc, char** argv)
{


    Data sdata;
    sdata.valid = false;
    sdata.mx = CreateMutex(NULL, false, NULL);
    memset(sdata.data, 0, sizeof(sdata.data));

    Data rdata;
    rdata.valid = false;
    rdata.mx = CreateMutex(NULL, false, NULL);
    memset(rdata.data, 0, sizeof(rdata.data));


    SocketParam sparam;
    strcpy_s(sparam.address, IP_ADDRESS);
    sparam.port = PORT_NUMBER;
    sparam.sdata = &sdata;
    sparam.rdata = &rdata;

    HANDLE hSocketTh = INVALID_HANDLE_VALUE;
    hSocketTh = CreateThread(NULL, 0, SocketThread, (LPVOID)&sparam, 0, NULL);


    HANDLE hInputTh = INVALID_HANDLE_VALUE;
    hInputTh = CreateThread(NULL, 0, InputThread, (LPVOID*)&sdata, 0, NULL);
    while (true) {
        Sleep(100);

        WaitForSingleObject(rdata.mx, INFINITE);
        if (rdata.valid) {
            printf("rdata : %s\n", rdata.data);
            rdata.valid = false;
        }
        ReleaseMutex(rdata.mx);
    }
    return 0;
}