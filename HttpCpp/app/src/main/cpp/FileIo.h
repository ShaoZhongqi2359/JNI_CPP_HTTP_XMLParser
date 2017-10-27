//
// Created by Administrator on 2017/10/17.
//

#ifndef HTTPCPP_FILEIO_H
#define HTTPCPP_FILEIO_H

#include <string>
#include <fstream>
#include <iostream>
#include<stdio.h>
#include <mutex>
#include <fcntl.h>

class FileIo {

public:

    static int writeContentToFile(const std::string &fileName, std::string &content) {
        std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        int fd = open(fileName.c_str(), O_CREAT | O_WRONLY,0666);
        if (fd == -1) {
            int errnum = errno;
            std::string errinfo = strerror(errno);
            //ALOGE("shao write file errorno = %d\n",errno);
            return -1;
        }
        //判断是不是xml文件
        size_t xmlPosition = content.find("<?xml");
        if (xmlPosition == std::string::npos) {
            return -1;
        }
        std::string result = content.substr(xmlPosition);
        write(fd, result.c_str(), result.length());
        close(fd);
        return 200;
    }

};


#endif //HTTPCPP_FILEIO_H
