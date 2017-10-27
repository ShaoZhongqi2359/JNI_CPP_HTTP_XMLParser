/*
 * LibLoader.h
 *
 *  Created on: 2016��7��5��
 *      Author: Steven
 */

#ifndef LIBLOADER_H_
#define LIBLOADER_H_

#include <cstdint>

typedef void (*httpGet)(char* url);

class LibLoader {
public:
	LibLoader();
	virtual ~LibLoader();

	void *getHandler(){
		return handle;
	}

	httpGet _httpGet;
private:
	void *handle;
};

#endif /* LIBLOADER_H_ */
