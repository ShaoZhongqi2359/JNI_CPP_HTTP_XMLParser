
#include "LibLoader.h"
#include <dlfcn.h>

LibLoader::LibLoader() {
    // TODO Auto-generated constructor stub
    handle = dlopen("/system/lib64/libhttp.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        //LOGE("open depend lib failed");
        handle = NULL;
    }
    _httpGet 	= (httpGet)dlsym(handle,"nibiru_http_get");
}

LibLoader::~LibLoader() {
    // TODO Auto-generated destructor stub
    if (handle) {
        dlclose(handle);
    }

    handle = NULL;
}

