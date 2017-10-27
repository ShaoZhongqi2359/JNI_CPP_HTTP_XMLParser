//
// Created by Administrator on 2017/10/23.
//

#include <iostream>
#include "NibiruXmlParser.h"

int NibiruXmlParser::loadXml() {
    XMLDocument doc;
    doc.LoadFile("/sdcard/shao.xml");

    XMLElement *rootElement = doc.RootElement();

    XMLElement *surface = rootElement->FirstChildElement("package");
    while (surface) {
        XMLElement *surfaceChild = surface->FirstChildElement();
        const char *content;
        const XMLAttribute *attributeOfSurface = surface->FirstAttribute();
        std::cout << attributeOfSurface->Name() << ":" << attributeOfSurface->Value() << std::endl;
        while (surfaceChild) {
            content = surfaceChild->GetText();
            surfaceChild = surfaceChild->NextSiblingElement();
            std::cout << content << std::endl;
        }
        surface = surface->NextSiblingElement();
    }

    return doc.ErrorID();
}

std::map<std::string, Config> NibiruXmlParser::parseConfigXml(const std::string filePath) {
    XMLDocument doc;
    doc.LoadFile(filePath.c_str());
    XMLElement *rootElement = doc.RootElement();
    XMLElement *packElement = rootElement->FirstChildElement("package");
    std::map<std::string, Config> vrConfigMap;
    vrConfigMap.clear();
    while (packElement) {
        //获得packageName
        Config modelConfig;
        XMLElement *packageChild = packElement->FirstChildElement();
        const XMLAttribute *attributeOfSurface = packElement->FirstAttribute();
        modelConfig.packageName = attributeOfSurface->Value();
        while (packageChild) {
            const std::string childName = packageChild->Name();
            std::string value;
            if (packageChild->GetText() == NULL) {
                value = "";
            } else {
                value = packageChild->GetText();
            }
            if (childName.compare("enable") == 0) {
                modelConfig.enable = value;
            } else if (childName.compare("scale") == 0) {
                modelConfig.scale = value;
            } else if (childName.compare("stretch") == 0) {
                modelConfig.stretch = value;
            } else if (childName.compare("offsetx") == 0) {
                modelConfig.offsetx = value;
            } else if (childName.compare("offsety") == 0) {
                modelConfig.offsety = value;
            } else if (childName.compare("overlayx") == 0) {
                modelConfig.overlayx = value;
            } else if (childName.compare("overlayy") == 0) {
                modelConfig.overlayy = value;
            } else if (childName.compare("mask") == 0) {
                modelConfig.showmask = value;
            } else if (childName.compare("category") == 0) {
                modelConfig.category = atoi(value.c_str());
            } else if (childName.compare("hwc") == 0) {
                modelConfig.dishwc = value;
            } else if (childName.compare("touch") == 0) {
                modelConfig.touch = value;
            } else if (childName.compare("sdk_distort") == 0) {
                modelConfig.sdk = value;
            }
            packageChild = packageChild->NextSiblingElement();
        }
        vrConfigMap.insert(std::pair<std::string, Config>(modelConfig.packageName, modelConfig));
        packElement = packElement->NextSiblingElement();
    }
    return vrConfigMap;
};