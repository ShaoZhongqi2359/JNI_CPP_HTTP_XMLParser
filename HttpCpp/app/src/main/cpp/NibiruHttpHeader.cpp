#include "NibiruHttpHeader.h"
#include <stdio.h>
#include "rechercheInformation.h"
#include <string.h>
#include <cstdlib>
#include <netdb.h>
#include <arpa/inet.h>

Header::Header()
{
    host = path = referer = userAgent = accept = accept_language = accept_encoding = cookie = variable = connexion = if_None_Match = "";
    method = "GET";
}
const char* Header::getHostChar(bool removeHttp)
{
    if(removeHttp)
        return host.c_str();
    else return std::string("http://"+host).c_str();
}

std::string Header::getHostString(bool removeHttp)
{
    if(removeHttp)
        return host;
    else return "http://"+host;
}

void Header::addVariable(std::string variable,std::string value)
{
    if(value.find(" ") != std::string::npos)
    {
        std::string tampon = value;
        RechercheInfo::searchCutRight(&tampon," ",true);
        RechercheInfo::searchCutLeft(&value," ",true);
        value=tampon+'+'+value;
    }

    if(this->variable.compare(""))
        this->variable+="&";

    this->variable += variable + "="+value;

}

void Header::addCookie(std::string variable,std::string value)
{
    if(!cookieExist(variable))
    {
       if(this->cookie.compare(""))
        {
            this->cookie+=" ";
        }
        this->cookie += variable+"="+value+';';

    }else{
        std::string tampon = cookie;
        int nb = tampon.find(variable+'=');
        std::string offset = variable+'=';
        RechercheInfo::searchCutLeft(&tampon,variable+'=',true);
        int nb2 = tampon.find(';');
        cookie.replace(nb+offset.size(),nb2,value);
    }

}

bool Header::cookieExist(std::string variable)
{
    std::string tampn=cookie;
    RechercheInfo::searchCutLeft(&tampn,variable+'=',true);
    if(tampn.size()==0)return false; else return true;
}

void Header::setHost(std::string host)
{
    removeHttp(&host);
    this->host = host;
}

void Header::setPath(std::string past)
{
    this->path = past;
    removeHttp(&past);
}

void Header::removeHttp(std::string* adresse)
{
    if(adresse->find("http://")==0)
        adresse->replace(0,7,"");
    else if(adresse->find("https://")==0)
        adresse->replace(0,8,"");
}

void Header::removeHttp(char adresse[])
{
    std::string dd=adresse;
    if(dd.find("http://")==0)
    {
        dd.replace(0,7,"");

        strcpy(adresse, dd.c_str());
    }else if(dd.find("https://")==0)
    {
        dd.replace(0,8,"");

        strcpy(adresse, dd.c_str());
    }
}

//返回address类似http://www.baidu.com:8080/
char *Header::GetHostAddrFromUrl(const char *strUrl) {
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

//返回address类似http://www.baidu.com
char *Header::GetHostAddrExclutePortFromUrl(const char *strUrl) {
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
        if (strAddr[i] == ':') {
            break;
        } else {
            strHostAddr[i] = strAddr[i];
        }
    }
    return strHostAddr;
}


char *Header::GetParamFromUrl(const char *strUrl) {
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
        strcpy(strParam, strAddr + iPos);
    }
    return strParam;
}


//从HTTP请求URL中获取端口号
int Header::GetPortFromUrl(const char *strUrl) {
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
char *Header::GetIPFromUrl(const char *strUrl) {
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

