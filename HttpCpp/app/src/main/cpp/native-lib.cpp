#include <jni.h>
#include <string>
#include "NibiruHttpRequest.h"
#include "LibLoader.h"
#include "NibiruHttpHeader.h"
#include "NibiruHttp.h"
#include "FileIo.h"
#include "NibiruXml/NibiruXmlParser.h"

LibLoader *libLoader;

#define BUFSIZE 57000
#define RETURN_VALUE_SIZE 410000

char *jstringTostring(JNIEnv *env, jstring jstr);

jstring charTojstring(JNIEnv *env, const char *pat);

extern "C"
JNIEXPORT jstring

JNICALL
Java_nibiruvr_nibiru_test_com_httpcpp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
//    std::string hello = "Hello from C++";
//
//    NibiruHttpRequest Http;
//    char http_return[RETURN_VALUE_SIZE] = {0};
//    char http_msg[4096] = {0};
//    strcpy(http_msg, "http://rom.1919game.net:8081/NibiruRom/app/CheckVRKeyMapAction?keymapTime=1502444523662&channel=VR0829&romVersion=3&version=1");
//    //http://rom.1919game.net:8081/NibiruRom/download/romkeymap/version/romkeymap_867.xml
//    //http://push.game1919.net:8080/NibiruPropell/checkpushmessage?packagename=3.22.001&uid=020f2c347174-38bc1a15bbb1&channel=VR0829&platform=rom
//    //http://rom.1919game.net:8080/NibiruRom/reg/RomRegCodeActivate?deviceId=020f2c347174-38bc1a15bbb1&code=0gyACvAPOe2ACRACkgQ%3D&channel=VR0829&romVersion=3&uid=38bc1a15bbb1
//    //strcpy(http_msg, "http://rom.1919game.net:8081/NibiruRom/download/romkeymap/version/romkeymap_867.xml");
//    //strcpy(http_msg, "http://rom.1919game.net:8080/NibiruRom/reg/RomRegCodeActivate?deviceId=020f2c347174-38bc1a15bbb1&code=0gyACvAPOe2ACRACkgQ%3D&channel=VR0829&romVersion=3&uid=38bc1a15bbb1");
//
//    if(Http.HttpGet(http_msg, http_return)){
//    }


    std::string URL(
            "http://rom.1919game.net:8080/NibiruRom/reg/RomRegCodeActivate?deviceId=020f2c347174-38bc1a15bbb1&code=0gyACvAPOe2ACRACkgQ%3D&channel=VR0829&romVersion=3&uid=38bc1a15bbb1");

    Header head;

    head.setHost(head.GetHostAddrExclutePortFromUrl(URL.c_str()));

    head.setPath(head.GetParamFromUrl(URL.c_str()));

    head.setPort(head.GetPortFromUrl(URL.c_str()));

    head.setMethod("GET");

    head.setAccept("text/html,application/xhtml+xml,application/xml;q=0.9,*//*;q=0.8");
    head.setUserAgent("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:21.0) Gecko/20100101 Firefox/21.0");
    head.setAccept_language("fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3");

    //head.setAccept_encoding("gzip, deflate");
    head.setConnexion("keep-alive");

    int value;
    Http http(head,&value);
    if (value == 0) {
        //return -1
    } else {
        std::string page = http.getPage(&head);
        FileIo::writeContentToFile("/sdcard/shao.xml", page);
    }

    //std::cout<<page<<std::endl;
    //falcutatif
    head.removeVariable();
    head.removeCookie();

    return env->NewStringUTF("122");
}

extern "C"
JNIEXPORT jint JNICALL Java_nibiruvr_nibiru_test_com_httpcpp_MainActivity_requestXmlInfo
        (JNIEnv *env, jobject obj, jstring url, jstring filePath) {

    std::string URL(env->GetStringUTFChars(url, 0));
    std::string FILE_PATH(env->GetStringUTFChars(filePath, 0));
    Header head;
    head.setHost(head.GetHostAddrExclutePortFromUrl(URL.c_str()));
    head.setPath(head.GetParamFromUrl(URL.c_str()));
    head.setPort(head.GetPortFromUrl(URL.c_str()));
    head.setMethod("GET");
    head.setAccept("text/html,application/xhtml+xml,application/xml;q=0.9,*//*;q=0.8");
    head.setUserAgent("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:21.0) Gecko/20100101 Firefox/21.0");
    head.setAccept_language("fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3");
    head.setConnexion("keep-alive");

    int value;
    Http http(head, &value);
    if (value == 0) {
        return -1;
    } else {
        std::string page = http.getPage(&head);
        if (page.empty()) {
            return -1;
        }
        int result = FileIo::writeContentToFile(FILE_PATH, page);
        head.removeVariable();
        head.removeCookie();
        return result;
    }

}

extern "C"
JNIEXPORT void
JNICALL Java_nibiruvr_nibiru_test_com_httpcpp_MainActivity_getHttpInfo
        (JNIEnv *env, jobject obj, jstring url) {
    libLoader = new LibLoader();
    libLoader->_httpGet(jstringTostring(env, url));
}

extern "C"
JNIEXPORT void JNICALL Java_nibiruvr_nibiru_test_com_httpcpp_MainActivity_parseXml
        (JNIEnv *env, jobject obj) {
    NibiruXmlParser testXmlParser;
    testXmlParser.parseConfigXml("");

}

extern "C"
JNIEXPORT jobject JNICALL Java_nibiruvr_nibiru_test_com_httpcpp_MainActivity_getConfigMapValue
        (JNIEnv *env, jobject obj,jstring filePath) {

    std::string FILE_PATH(env->GetStringUTFChars(filePath, 0));
    NibiruXmlParser testXmlParser;
    std::map<std::string, Config> nativeMap = testXmlParser.parseConfigXml(FILE_PATH);
    jobject javaHashMap;
    if (nativeMap.size() > 0) {
        //创建java层的hashmap
        jclass class_Java_HashMap = env->FindClass("java/util/HashMap");
        jmethodID java_hashMap_Init = env->GetMethodID(class_Java_HashMap, "<init>", "()V");
        javaHashMap = env->NewObject(class_Java_HashMap, java_hashMap_Init, "");
        jmethodID HashMap_put = env->GetMethodID(class_Java_HashMap, "put",
                                                 "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        jclass java_Config_Class = env->FindClass("nibiruvr/nibiru/test/com/httpcpp/Config");
        jmethodID initMethodID = env->GetMethodID(java_Config_Class, "<init>", "()V");
        jfieldID field_name = env->GetFieldID(java_Config_Class, "packageName",
                                              "Ljava/lang/String;");
        jfieldID field_enable = env->GetFieldID(java_Config_Class, "configEnable",
                                                "Ljava/lang/String;");
        jfieldID field_scale = env->GetFieldID(java_Config_Class, "scale", "Ljava/lang/String;");
        jfieldID field_stretch = env->GetFieldID(java_Config_Class, "stretch",
                                                 "Ljava/lang/String;");
        jfieldID field_offsetx = env->GetFieldID(java_Config_Class, "offsetx",
                                                 "Ljava/lang/String;");
        jfieldID field_offsety = env->GetFieldID(java_Config_Class, "offsety",
                                                 "Ljava/lang/String;");
        jfieldID field_overlayx = env->GetFieldID(java_Config_Class, "overlayx",
                                                  "Ljava/lang/String;");
        jfieldID field_overlayy = env->GetFieldID(java_Config_Class, "overlayy",
                                                  "Ljava/lang/String;");
        jfieldID field_mask = env->GetFieldID(java_Config_Class, "showmask", "Ljava/lang/String;");
        jfieldID field_category = env->GetFieldID(java_Config_Class, "category", "I");
        jfieldID field_hwc = env->GetFieldID(java_Config_Class, "dishwc", "Ljava/lang/String;");
        jfieldID field_touch = env->GetFieldID(java_Config_Class, "touch", "Ljava/lang/String;");
        jfieldID field_sdk = env->GetFieldID(java_Config_Class, "sdk", "Ljava/lang/String;");

        //进行迭代
        for (std::map<std::string, Config>::iterator it = nativeMap.begin();
             it != nativeMap.end(); it++) {
            std::string configPackageName = it->first;
            Config nativeConfig = it->second;
            jstring packageName;

            //创建Java层对象
            jobject javaConfig = env->NewObject(java_Config_Class, initMethodID);
            if (configPackageName.c_str() != NULL && configPackageName.length() > 0) {
                packageName = (env)->NewStringUTF(configPackageName.c_str());
                (env)->SetObjectField(javaConfig, field_name, packageName);
            }

            jstring fieldEnale = (env)->NewStringUTF(nativeConfig.enable.c_str());
            jstring fieldScale = (env)->NewStringUTF(nativeConfig.scale.c_str());
            jstring fieldStrech = (env)->NewStringUTF(nativeConfig.stretch.c_str());
            jstring fieldOffsetx = (env)->NewStringUTF(nativeConfig.offsetx.c_str());
            jstring fieldOffsety = (env)->NewStringUTF(nativeConfig.offsety.c_str());
            jstring fieldOverlayx = (env)->NewStringUTF(nativeConfig.overlayx.c_str());
            jstring fieldOverlayy = (env)->NewStringUTF(nativeConfig.overlayy.c_str());
            jstring fieldShowmax = (env)->NewStringUTF(nativeConfig.showmask.c_str());
            jstring fieldDishwc = (env)->NewStringUTF(nativeConfig.dishwc.c_str());
            jstring fieldTouch = (env)->NewStringUTF(nativeConfig.touch.c_str());
            jstring fieldSdk = (env)->NewStringUTF(nativeConfig.sdk.c_str());


            (env)->SetObjectField(javaConfig, field_enable, fieldEnale);
            (env)->SetObjectField(javaConfig, field_scale, fieldScale);
            (env)->SetObjectField(javaConfig, field_stretch, fieldStrech);
            (env)->SetObjectField(javaConfig, field_offsetx, fieldOffsetx);
            (env)->SetObjectField(javaConfig, field_offsety, fieldOffsety);
            (env)->SetObjectField(javaConfig, field_overlayx, fieldOverlayx);
            (env)->SetObjectField(javaConfig, field_overlayy, fieldOverlayy);
            (env)->SetObjectField(javaConfig, field_mask, fieldShowmax);
            (env)->SetIntField(javaConfig, field_category, (jint) (nativeConfig.category));
            (env)->SetObjectField(javaConfig, field_hwc, fieldDishwc);
            (env)->SetObjectField(javaConfig, field_touch, fieldTouch);
            (env)->SetObjectField(javaConfig, field_sdk, fieldSdk);

            env->CallObjectMethod(javaHashMap, HashMap_put, packageName, javaConfig);
            env->DeleteLocalRef(javaConfig);
            env->DeleteLocalRef(packageName);

            env->DeleteLocalRef(fieldEnale);
            env->DeleteLocalRef(fieldScale);
            env->DeleteLocalRef(fieldStrech);
            env->DeleteLocalRef(fieldOffsetx);
            env->DeleteLocalRef(fieldOffsety);
            env->DeleteLocalRef(fieldOverlayx);
            env->DeleteLocalRef(fieldOverlayy);
            env->DeleteLocalRef(fieldShowmax);
            env->DeleteLocalRef(fieldDishwc);
            env->DeleteLocalRef(fieldTouch);
            env->DeleteLocalRef(fieldSdk);
        }
    }
    return javaHashMap;
}

char *jstringTostring(JNIEnv *env, jstring jstr) {
    char *rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte *ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char *) malloc(alen + 1);

        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

jstring charTojstring(JNIEnv *env, const char *pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte *) pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("utf-8");
    //将byte数组转换为java String,并输出
    return (jstring) (env)->NewObject(strClass, ctorID, bytes, encoding);
}
