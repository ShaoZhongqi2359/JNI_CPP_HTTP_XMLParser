#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <string>
#include "NibiruHttpHeader.h"


#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


class Http {
public:
    Http(Header head, int *result);

    ~Http();

    //get the body and head of the page
    std::string getPage(Header *head);

private:

    int ConnectSocket();

    std::string recvPaquet();

    int CreateSocket(Header head);

    unsigned int tailleHead(std::string page);

    int hexaTodecimal(std::string hexa);

    SOCKET sock;
    SOCKADDR_IN sin;

    void redirection(Header *head, std::string *reponce);

    int recvTimeOut(unsigned int s, int millisecond, std::string *chaine);


    void sendPaquet(Header head);

    void getCookie(Header *head, std::string reponce);

};
