//
// Created by Administrator on 2017/10/10.
//

#include "NibiruHttpRequest.h"
#include <poll.h>


NibiruHttpRequest::NibiruHttpRequest() {

}

NibiruHttpRequest::~NibiruHttpRequest() {

}

int NibiruHttpRequest::HttpGet(const char *strUrl, char *strResponse) {
    int result = HttpRequestExec("GET", strUrl, NULL, strResponse);
    if (result == 2) {
        //重定向再次发送请求
        if (mUrl302 != NULL && strlen(mUrl302) > 0) {
            result = HttpRequestExec("GET", mUrl302, NULL, strResponse);
            if (mUrl302 != NULL) {
                free(mUrl302);
            }
            return result;
        }
    } else {
        return result;
    }
    return result;
}


/**
* 1表示成功
* 0表示失败
* 2表示302重定向
**/
int NibiruHttpRequest::HttpPost(const char *strUrl, const char *strData, char *strResponse) {
    return HttpRequestExec("POST", strUrl, strData, strResponse);
}


//执行HTTP请求，GET或POST
int
NibiruHttpRequest::HttpRequestExec(const char *strMethod, const char *strUrl, const char *strData,
                                   char *strResponse) {
    //判断URL是否有效
    if ((strUrl == NULL) || (0 == strcmp(strUrl, ""))) {
        DebugOut("%s %s %d\tURL为空\n", __FILE__, __FUNCTION__, __LINE__);
        return 0;
    }

    //限制URL长度
    if (URLSIZE < strlen(strUrl)) {
        DebugOut("%s %s %d\tURL的长度不能超�?d\n", __FILE__, __FUNCTION__, __LINE__, URLSIZE);
        return 0;
    }

    //创建请求头
    char *strHttpHead = HttpHeadCreate(strMethod, strUrl, strData);

    //判断套接字m_iSocketFd是否有效，有效就直接发送数据?
    if (m_iSocketFd != INVALID_SOCKET) {
        //检查SocketFd是否为可写不可读状�?
        if (SocketFdCheck(m_iSocketFd) > 0) {
            char *strResult = HttpDataTransmit(strHttpHead, m_iSocketFd);
            if (NULL != strResult) {
                strcpy(strResponse, strResult);
                return 1;
            }
        }
    }

    //Create socket
    m_iSocketFd = INVALID_SOCKET;
    m_iSocketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_iSocketFd < 0) {
        //ALOGE("shao create socket wrong check network\n");
        return 0;
    }

    //Bind address and port
    int iPort = GetPortFromUrl(strUrl);
    if (iPort < 0) {
        //ALOGE("shao get port from url wrong check network\n");
        return 0;
    }
    char *strIP = GetIPFromUrl(strUrl);
    if (strIP == NULL) {
        //ALOGE("shao get ip address from url wrong check network\n");
        return 0;
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(iPort);
    if (inet_pton(AF_INET, strIP, &servaddr.sin_addr) <= 0) {
        close(m_iSocketFd);
        m_iSocketFd = INVALID_SOCKET;
        return 0;
    }

    //Set non-blocking
    int flags = fcntl(m_iSocketFd, F_GETFL, 0);
    if (fcntl(m_iSocketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(m_iSocketFd);
        m_iSocketFd = INVALID_SOCKET;
        return 0;
    }
    int iRet = connect(m_iSocketFd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    //ALOGD("shao connect result = %d\n",iRet);
    if (iRet == 0) {
        char *strResult = HttpDataTransmit(strHttpHead, m_iSocketFd);
        if (NULL != strResult) {
            strcpy(strResponse, strResult);
            //ALOGD("shao connect return value = %s\n",strResponse);
            free(strResult);
            return 1;
        } else {
            close(m_iSocketFd);
            m_iSocketFd = INVALID_SOCKET;
            free(strResult);
            return 0;
        }
    } else if (iRet < 0) {
        //ALOGD("shao socket error check fd");
        if (errno != EINPROGRESS) {
            return 0;
        }
    }

    iRet = SocketFdCheck(m_iSocketFd);
    if (iRet > 0) {
        char *strResult = HttpDataTransmit(strHttpHead, m_iSocketFd);
        if (NULL == strResult) {
            close(m_iSocketFd);
            m_iSocketFd = INVALID_SOCKET;
            return 0;
        } else {
            //302再次发送请求
            if (Check302Data(strResult)) {
                mUrl302 = Get302Url(strResult);
                if (strResult != NULL) {
                    free(strResult);
                }
                m_iSocketFd = INVALID_SOCKET;
                return 2;
            } else {
                strcpy(strResponse, strResult);
                //ALOGD("shao checksocket return value = %s\n",strResponse);
                free(strResult);
                m_iSocketFd = INVALID_SOCKET;
                return 1;
            }
        }
    } else {
        close(m_iSocketFd);
        m_iSocketFd = INVALID_SOCKET;
        return 0;
    }

    return 1;
}

char *
NibiruHttpRequest::HttpHeadCreate(const char *strMethod, const char *strUrl, const char *strData) {
    char *strHost = GetHostAddrFromUrl(strUrl);
    char *strParam = GetParamFromUrl(strUrl);

    char *strHttpHead = (char *) malloc(BUFSIZE);
    memset(strHttpHead, 0, BUFSIZE);

    strcat(strHttpHead, strMethod);
    strcat(strHttpHead, " /");
    strcat(strHttpHead, strParam);
    free(strParam);
    strcat(strHttpHead, " HTTP/1.1\r\n");
    strcat(strHttpHead, "Accept-Encoding: gzip,deflate\r\n");
    strcat(strHttpHead, "User-Agent: Apache-HttpClient/4.1.1\r\n");
    strcat(strHttpHead, "Host: ");
    strcat(strHttpHead, strHost);
    strcat(strHttpHead, "\r\n");
    strcat(strHttpHead, "Connection: Keep-Alive\r\n");
    if (0 == strcmp(strMethod, "POST")) {
        char len[8] = {0};
        unsigned uLen = strlen(strData);
        sprintf(len, "%d", uLen);

        strcat(strHttpHead, "Content-Type: application/x-www-form-urlencoded\r\n");
        strcat(strHttpHead, "Content-Length: ");
        strcat(strHttpHead, len);
        strcat(strHttpHead, "\r\n\r\n");
        strcat(strHttpHead, strData);
    }
    strcat(strHttpHead, "\r\n\r\n");

    free(strHost);

    return strHttpHead;
}


char *NibiruHttpRequest::HttpDataTransmit(char *strHttpHead, const int iSockFd) {

    int ret = send(iSockFd, (void *) strHttpHead, strlen(strHttpHead) + 1, 0);
    free(strHttpHead);
    if (ret < 0) {
        DebugOut("%s %s %d\tsend error! Error code: %d，Error message: %s\n", __FILE__, __FUNCTION__,
                 __LINE__, errno, strerror(errno));
        close(iSockFd);
        return NULL;
    }

    return PollFdData(iSockFd);
}


char *NibiruHttpRequest::GetHostAddrFromUrl(const char *strUrl) {
    char url[URLSIZE] = {0};
    strcpy(url, strUrl);

    char *strAddr = strstr(url, "http://");//判断有没有http://
    if (strAddr == NULL) {
        strAddr = strstr(url, "https://");//判断有没有https://
        if (strAddr != NULL) {
            strAddr += 8;
        }
    } else {
        strAddr += 7;
    }

    if (strAddr == NULL) {
        strAddr = url;
    }
    int iLen = strlen(strAddr);
    char *strHostAddr = (char *) malloc(iLen + 1);
    memset(strHostAddr, 0, iLen + 1);
    for (int i = 0; i < iLen + 1; i++) {
        if (strAddr[i] == '/') {
            break;
        } else {
            strHostAddr[i] = strAddr[i];
        }
    }

    return strHostAddr;
}

char *NibiruHttpRequest::GetParamFromUrl(const char *strUrl) {
    char url[URLSIZE] = {0};
    strcpy(url, strUrl);

    char *strAddr = strstr(url, "http://");//判断有没有http://
    if (strAddr == NULL) {
        strAddr = strstr(url, "https://");//判断有没有https://
        if (strAddr != NULL) {
            strAddr += 8;
        }
    } else {
        strAddr += 7;
    }

    if (strAddr == NULL) {
        strAddr = url;
    }
    int iLen = strlen(strAddr);
    char *strParam = (char *) malloc(iLen + 1);
    memset(strParam, 0, iLen + 1);
    int iPos = -1;
    for (int i = 0; i < iLen + 1; i++) {
        if (strAddr[i] == '/') {
            iPos = i;
            break;
        }
    }
    if (iPos == -1) {
        strcpy(strParam, "");;
    } else {
        strcpy(strParam, strAddr + iPos );
    }
    return strParam;
}


//从HTTP请求URL中获取端口号
int NibiruHttpRequest::GetPortFromUrl(const char *strUrl) {
    int iPort = -1;
    char *strHostAddr = GetHostAddrFromUrl(strUrl);
    if (strHostAddr == NULL) {
        return -1;
    }

    char strAddr[URLSIZE] = {0};
    strcpy(strAddr, strHostAddr);
    free(strHostAddr);

    char *strPort = strchr(strAddr, ':');
    if (strPort == NULL) {
        iPort = 80;
    } else {
        iPort = atoi(++strPort);
    }
    return iPort;
}


//从HTTP请求URL中获取IP地址
char *NibiruHttpRequest::GetIPFromUrl(const char *strUrl) {
    char *strHostAddr = GetHostAddrFromUrl(strUrl);
    int iLen = strlen(strHostAddr);
    char *strAddr = (char *) malloc(iLen + 1);
    memset(strAddr, 0, iLen + 1);
    int iCount = 0;
    int iFlag = 0;
    for (int i = 0; i < iLen + 1; i++) {
        if (strHostAddr[i] == ':') {
            break;
        }

        strAddr[i] = strHostAddr[i];
        if (strHostAddr[i] == '.') {
            iCount++;
            continue;
        }
        if (iFlag == 1) {
            continue;
        }

        if ((strHostAddr[i] >= '0') || (strHostAddr[i] <= '9')) {
            iFlag = 0;
        } else {
            iFlag = 1;
        }
    }
    free(strHostAddr);

    if (strlen(strAddr) <= 1) {
        return NULL;
    }

    //判断是否为点分十进制IP地址，否则通过域名地址获取IP地址
    if ((iCount == 3) && (iFlag == 0)) {
        return strAddr;
    } else {
        //"rom.1919game.net"
        struct hostent *he = gethostbyname(strAddr);
        free(strAddr);
        if (he == NULL) {
            return NULL;
        } else {
            struct in_addr **addr_list = (struct in_addr **) he->h_addr_list;
            for (int i = 0; addr_list[i] != NULL; i++) {
                return inet_ntoa(*addr_list[i]);
            }
            return NULL;
        }
    }
}

//判断返回数据是不是302重定向的指令
bool NibiruHttpRequest::Check302Data(const char *responseData) {
    if (responseData == NULL || strlen(responseData) <= 0) {
        return false;
    } else if (strstr(responseData, "302") != NULL && strstr(responseData, "Location") != NULL) {
        return true;
    } else {
        return false;
    }
}

//获取302重定向新的URL
char *NibiruHttpRequest::Get302Url(char *responseData) {
    if (responseData == NULL || strlen(responseData) <= 0) {
        return NULL;
    }
    char *strAddr = strstr(responseData, "http://");//判断有没有http://
    if (strAddr == NULL) {
        return NULL;
    }
    char *endAddr = strchr(strAddr, '\r\n');

    int urlLength = endAddr - strAddr - 1;

    char *newUrl = (char *) malloc(urlLength + 1);
    memset(newUrl, 0, urlLength + 1);

    strncpy(newUrl, strAddr, urlLength);
    return newUrl;
}


//检查SocketFd是否为可写不可读状态
int NibiruHttpRequest::SocketFdCheck(const int iSockFd) {
    struct timeval timeout;
    fd_set rset, wset;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(iSockFd, &rset);
    FD_SET(iSockFd, &wset);
    timeout.tv_sec = 3;
    timeout.tv_usec = 500;
    int iRet = select(iSockFd + 1, &rset, &wset, NULL, &timeout);
    if (iRet > 0) {
        //判断SocketFd是否为可写不可读状�?
        int iW = FD_ISSET(iSockFd, &wset);
        int iR = FD_ISSET(iSockFd, &rset);
        if (iW && !iR) {
            char error[4] = "";
            socklen_t len = sizeof(error);
            int ret = getsockopt(iSockFd, SOL_SOCKET, SO_ERROR, error, &len);
            if (ret == 0) {
                if (!strcmp(error, "")) {
                    return iRet;//表示已经准备好的描述符数
                }
            }
        }
    } else if (iRet == 0) {
        return 0;//表示超时
    } else {
        return -1;//select出错，所有描述符集清0
    }
    return -2;//其他错误
}


//通过Select方式去接受数据

char *NibiruHttpRequest::ReceiveFdData(const int iSockFd) {
    char *buf = (char *) malloc(BUFSIZE);
    memset(buf, 0, BUFSIZE);

    char *resultValue = (char *) malloc(BUFSIZE);
    memset(resultValue, 0, BUFSIZE);

    struct timeval timeout;
    fd_set rset, wset;
    int result;
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(iSockFd, &rset);
    FD_SET(iSockFd, &wset);

    timeout.tv_sec = 5;
    timeout.tv_usec = 500;
    //ALOGE("shao error 66");
    result = select(iSockFd + 1, &rset, &wset, NULL, &timeout);
    if (result > 0) {
        while (1) {
            int count = recv(iSockFd, buf, BUFSIZE, MSG_WAITALL);
            if (count < 0) {
                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
                    continue;
                } else {
                    break;
                }
            } else if (count == 0) {
                break;
            } else if (count > 0) {
                strcat(resultValue, buf);
                //ALOGE("shao get value = %s\n", buf);
                bzero(buf, BUFSIZE);
            }
        }
    }
    return resultValue;
}


char *NibiruHttpRequest::PollFdData(const int iSockFd) {

    char *buf = (char *) malloc(BUFSIZE);
    memset(buf, 0, BUFSIZE);

    char *resultValue = (char *) malloc(BUFSIZE);
    memset(resultValue, 0, BUFSIZE);

    int ret;
    int num, numread;
    struct pollfd poll_fd;
    memset(&poll_fd, '\0', sizeof(poll_fd));
    poll_fd.fd = iSockFd;
    poll_fd.events |= POLLIN;
    ret = 0;
    num = poll(&poll_fd, 1, -1);
    if(num  == -1){
      //有错误
      return resultValue;
    }else if(num == 0){
      //超时
      return resultValue;
    }else{
        if (poll_fd.revents & POLLIN) {
            while(1){
                numread = read(iSockFd, buf, BUFSIZE);
                if(numread <= 0){
                    break;
                }
                strcat(resultValue, buf);
                //ALOGE("shao get value = %s\n", buf);

                bzero(buf,BUFSIZE);
            }
        }
    }

    return resultValue;
}


//打印输出
void NibiruHttpRequest::DebugOut(const char *fmt, ...) {
#ifdef __DEBUG__
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
#endif
}