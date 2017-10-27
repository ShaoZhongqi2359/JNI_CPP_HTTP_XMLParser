//
// Created by Zhongqi.Shao on 2017/10/10.
//

#ifndef HTTPCPP_NIBIRUHTTPREQUEST_H
#define HTTPCPP_NIBIRUHTTPREQUEST_H


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFSIZE 56500
#define RETURN_VALUE_SIZE 410000
#define URLSIZE 1024
#define INVALID_SOCKET -1
#define __DEBUG__


class NibiruHttpRequest {

public:

    enum MODE{
        GET,
        POST
    };

    NibiruHttpRequest();

    ~NibiruHttpRequest();

    void DebugOut(const char *fmt, ...);

    int HttpGet(const char *strUrl, char *strResponse);

    int HttpPost(const char *strUrl, const char *strData, char *strResponse);

private:
    int HttpRequestExec(const char *strMethod, const char *strUrl, const char *strData,
                        char *strResponse);

    char *HttpHeadCreate(const char *strMethod, const char *strUrl, const char *strData);

    char *HttpDataTransmit(char *strHttpHead, const int iSockFd);

    int GetPortFromUrl(const char *strUrl);

    char *GetIPFromUrl(const char *strUrl);

    char *GetParamFromUrl(const char *strUrl);

    char *GetHostAddrFromUrl(const char *strUrl);

    bool Check302Data(const char *responseData); /*true 表示302  false 表示NO*/
    char *Get302Url(char *responseData);   //获取302新的URL

    int SocketFdCheck(const int iSockFd);

    char* ReceiveFdData(const int iSockFd);

    char* PollFdData(const int iSockFd);


    int m_iSocketFd = INVALID_SOCKET;


    char* mUrl302 = NULL;


};


#endif //HTTPCPP_NIBIRUHTTPREQUEST_H
