#include "NibiruHttp.h"
#include "rechercheInformation.h"
#include <sstream>

//TODO DEV HTTPS

#define BUFLEN 20480

Http::Http(Header head, int *result) {
    *result = CreateSocket(head);
    *result = ConnectSocket();
}

int Http::CreateSocket(Header head) {

    struct hostent *hostinfo = NULL;
    const char *hostname = head.getHostChar(true);
    hostinfo = gethostbyname(hostname);
    if (hostinfo == NULL) {
        fprintf(stderr, "Unknown host %s\n", hostname);
        return 0;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(head.getPort());
    return 1;

}

int Http::ConnectSocket() {
    if (connect(sock, (SOCKADDR *) &sin, sizeof(sin)) == SOCKET_ERROR) {
        //std::cout << "Error in the connection..." << std::endl;
        return 0;
    }
    return 1;
}

//reponce-> message renvoyer par le server dont on veut prendre les cookies.
void Http::getCookie(Header *head, std::string reponce) {

    RechercheInfo::searchCutLeft(&reponce, "Set-Cookie: ", true);
    std::string tampon = reponce;

    while (tampon.size() != 0) {
        std::string tampon2 = tampon;
        RechercheInfo::searchCutRight(&tampon2, "\r\n", false);
        RechercheInfo::searchCutRight(&tampon, "=", true);
        RechercheInfo::searchCutLeft(&tampon2, "=", true);

        if (tampon2.find(";") <= tampon2.size())
            RechercheInfo::searchCutRight(&tampon2, ";", true);
        else
            RechercheInfo::searchCutRight(&tampon2, "\r\n", true);

        head->addCookie(tampon, tampon2);
        RechercheInfo::searchCutLeft(&reponce, "Set-Cookie: ", true);
        tampon = reponce;
    }
}

std::string Http::getPage(Header *head) {

    sendPaquet(*head);

    std::string reponce = recvPaquet();

    getCookie(head, reponce);
    head->setReferer("http://" + head->getHostString(true) + head->getPathString());
    redirection(head, &reponce);

    return reponce;
}

void Http::redirection(Header *head, std::string *reponce) {
    if (reponce->find("\r\nlocation: ") != std::string::npos || reponce->find("\r\nLocation: ") !=
                                                                std::string::npos)//TODO changer imagine que sur le content y a un \r\nlocation
    {

        if (reponce->find("\r\nlocation: ") != std::string::npos)
            RechercheInfo::searchCutLeft(reponce, "\r\nlocation: ", true);


        if (reponce->find("\r\nLocation: ") != std::string::npos)
            RechercheInfo::searchCutLeft(reponce, "\r\nLocation: ", true);

        RechercheInfo::searchCutRight(reponce, "\r\n", true);

        if (reponce->find("http://") == std::string::npos)
            head->setPath(*reponce);
        else {
            std::string host = "";
            std::string path = "/";

            RechercheInfo::searchCutLeft(reponce, "http://", true);

            if (reponce->find("/") == std::string::npos)
                host = *reponce;
            else {
                path = *reponce;
                host = *reponce;

                RechercheInfo::searchCutLeft(&path, "/", false);
                RechercheInfo::searchCutRight(&host, "/", true);
            }

            head->setHost("http://" + host);
            head->setPath(path);
        }
        head->setMethod("GET");

        head->removeVariable();

        (*reponce) = getPage(head);
    }
}

void Http::sendPaquet(Header head) {
    //http protocol uses \r\n as a new line, and the last line must always be empty.
    std::string requete = head.getMethod() + " ";

    if (!head.getPathString().compare("")) {
        requete += "/ ";
    } else requete += head.getPathString();

    requete += " HTTP/1.1\r\n";

    if (head.getHostString().compare(""))
        requete += "Host: " + head.getHostString() + "\r\n";

    if (head.getUserAgent().compare(""))
        requete += "User-Agent: " + head.getUserAgent() + "\r\n";

    if (head.getAccept().compare(""))
        requete += "Accept: " + head.getAccept() + "\r\n";

    if (head.getAccept_language().compare(""))
        requete += "Accept-Language: " + head.getAccept_language() + "\r\n";

    if (head.getAccept_encoding().compare(""))
        requete += "Accept-Encoding: " + head.getAccept_encoding() + "\r\n";

    if (head.getRefererString().compare(""))
        requete += "Referer: " + head.getRefererString() + "\r\n";

    if (head.getCookie().compare(""))
        requete += "Cookie: " + head.getCookie() + "\r\n";
    requete += "Cookie: " + head.getCookie() + "\r\n";

    if (head.getConnexion().compare(""))
        requete += "Connection: " + head.getConnexion() + "\r\n";

    if (head.getIfnotMatch().compare(""))
        requete += "If-None-Match: " + head.getIfnotMatch() + "\r\n";

    if (!head.getVariable().compare("")) {
        requete += "\r\n";
    } else {
        std::ostringstream oss;
        oss << head.getVariable().size();
        requete +=
                "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: " + oss.str() +
                "\r\n\r\n" + head.getVariable() + "\r\n\r\n";
    }

    char *bufferOutput = new char[requete.length() + 1];
    strcpy(bufferOutput, requete.c_str());

    unsigned int nbEnvoyer = 0;
    int erreur = -1;
    while (nbEnvoyer < strlen(bufferOutput)) {
        erreur = send(sock, bufferOutput + nbEnvoyer, strlen(bufferOutput) - nbEnvoyer, 0);
        nbEnvoyer += erreur;
        if (erreur == 0 || erreur == -1) {
            std::cout
                    << "Erreur dans l'envois de la requette...La connexion a t elle pas été fermé?. "
                    << erreur << std::endl;//TODO GERER UNE RECONNEXION
        }
    }

    delete[] bufferOutput;
}

int Http::recvTimeOut(unsigned int sock, int millisecond, std::string *chaine) {
    struct timeval timeout;
    timeout.tv_sec = millisecond;
    timeout.tv_usec = 0;

    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(sock, &readfs);
    int nb = select(sock + 1, &readfs, NULL, NULL, &timeout);

    if (nb == 0) {
        return 0;
    } else if (nb == -1) {
        //CreateSocket(head);
        ConnectSocket();
        return 0;
    } else {
        char recvbuf[BUFLEN];
        int recvbuflen = BUFLEN;

        int result = recv(sock, recvbuf, recvbuflen, 0);
        for (int i = 0; i < result; i++)
            (*chaine) += recvbuf[i];

        return result;
    }
}

int Http::hexaTodecimal(std::string hex) {
    int x;
    std::istringstream iss(hex);
    iss >> std::hex >> x;
    return x;
}

unsigned int Http::tailleHead(std::string page) {
    int taille = page.length();
    RechercheInfo::searchCutRight(&page, "\r\n\r\n", false);

    if (page.length() == 0)
        return taille;

    return page.length();
}

std::string Http::recvPaquet() {
    //stock the number of bytes received
    unsigned int nbRecu = 0;
    //the number of bytes of the chunk.
    unsigned int chunked = 0;
    bool isChunked = false;
    bool haveContentLength = false;
    //the number of bytes of the content.
    unsigned int contentLenght = 0;
    //Stock the reponse
    std::string input = "";
    bool isRunning = true;
    unsigned int tampon;
    while (isRunning) {
        tampon = recvTimeOut(sock, 2000, &input);
        nbRecu += tampon;
        if (tampon == 0) {
            break;
        }
        if (!isChunked && input.find("Transfer-Encoding: chunked") != std::string::npos &&
            input.find("Transfer-Encoding: chunked") < tailleHead(input)) {
            std::string tampon = input;
            tampon.replace(nbRecu, tampon.size(), "");
            RechercheInfo::searchCutLeft(&tampon, "\r\n\r\n", true);
            RechercheInfo::searchCutRight(&tampon, "\r", true);
            if (tampon.size() > 0) {
                chunked = hexaTodecimal(tampon) + input.find(tampon) + tampon.size() + 4;
                isChunked = true;
                input.replace(input.find(tampon), tampon.size() + 2, "");
                nbRecu -= (tampon.length() + 2);
                chunked -= (tampon.length() + 2);
            }
        }

        while (isChunked && chunked < nbRecu) {
            std::string tampon = input;
            tampon.replace(0, chunked, "");
            RechercheInfo::searchCutRight(&tampon, "\r", true);
            //POUR GERER LE cas ou le input s'arrete pil sur la valeur hexa
            if (tampon.length() == 0)
                break;
            int nombre = hexaTodecimal(tampon);
            //if the size of the new chunk is 0 we stop.
            if (nombre == 0) {
                isRunning = false;
                break;
            } else //else we upgrade the chunk
            {
                int EmplacementChunked = 0;
                if (input.find(tampon) != std::string::npos)
                    EmplacementChunked = input.find(tampon);

                if (nbRecu < (EmplacementChunked + tampon.length() + 2)) break;

                input.replace(EmplacementChunked - 2, tampon.length() + 4, "X");

                nbRecu -= (tampon.length() + 3);
                chunked += nombre + 1;// en faite ce serait plus : -2 +2 + nombre
            }
        }

        if (!haveContentLength) {
            std::string tampon = input;
            //on garde uniquement le header
            tampon.replace(tailleHead(input), tampon.size() - tailleHead(input), "");

            RechercheInfo::searchCutLeft(&tampon, "Content-Length:", true);
            RechercheInfo::searchCutRight(&tampon, "\r", true);
            if (tampon.size() > 0) {
                std::istringstream iss(tampon);
                iss >> contentLenght;
                haveContentLength = true;
            }
        }
        if (haveContentLength && (contentLenght + tailleHead(input)) <= nbRecu) {
            break;
        }
    }
    return input;
}

Http::~Http() {
    closesocket(sock);
}

