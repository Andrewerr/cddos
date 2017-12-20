//
//  socket.c
//  cdos
//
//  Created by Andre Zay on 17.12.2017.
//  Copyright © 2017 Andre Zay. All rights reserved.
//

#include "socket.h"


int hostname2ip(char* hostname, char* ip)
{
    struct hostent* he;
    struct in_addr** addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) {
        // get the host info
        error("Failed to find host %s", hostname);
        return 1;
    }

    addr_list = (struct in_addr**)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        return 0;
    }

    return 1;
}
int dos_tcp_sock(char* _host, int port)
{
    int sock;
    char* host = (char*)malloc(sizeof(char) * 14);
    hostname2ip(_host, host);
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
#ifdef DEBUG
    error("\n\nWaiting is %d\n\n",socket_wait);
#endif
    if (sock < 0) {
        error("Could not create socket.Is everything ok with your system?");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
    if (connect(sock,(struct sockaddr *) &addr,sizeof(addr))  < 0) {
        error("Could not connect to server.");
        close(sock);
        return -2;
    }
    return sock;
}

char* dos_tcp_send(int sock, char* data)
{
    send(sock, data, strlen(data), 0);
    char* buf = (char*)malloc(sizeof(char) * 1024);
    memset(buf, '\0', 1024);
    if(socket_wait){
        if (recv(sock, buf, 1024, 0) < 0) {
            warning("Failed to recieve data from tcp host");
            return NULL;
        }
    }
    return buf;
}

int dos_udp_sock()
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        error("Could not create socket.Is everything ok with your system?");
        return -1;
    }
    struct timeval tv;
    if(socket_wait){
        tv.tv_sec = 1;
        tv.tv_usec = 100000;
    }else{
        tv.tv_usec=100;
    }
    if(socket_wait){
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            error("Failed to set socket otrions");
            return -2;
        }
    }
#ifdef DEBUG
    info("Created socket");
#endif
    return sock;
}
char* dos_udp_send(int sock, char* _host, int port, char* message)
{
    struct sockaddr_in addr;
    char* host = (char*)malloc(sizeof(char) * 14);
    hostname2ip(_host, host);
    char* buf = (char*)malloc(1024 * sizeof(char));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);
#ifdef DEBUG
    info("Attempting to send data");
#endif
    if (sendto(sock, message, strlen(message), 0, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        error("UDP sendto %s:%d failed.Maybe something wrong with host?", host, port);
        return NULL;
    }

    size_t slen = sizeof(addr);
    if (recvfrom(sock, buf, 1024, MSG_PEEK, (struct sockaddr*)&addr, &slen) == -1) {
        warning("UDP recvfrom %s failed.", host);
        return NULL;
    }
#ifdef DEBUG
    info("returning recieved buffer");
#endif
    return buf;
}