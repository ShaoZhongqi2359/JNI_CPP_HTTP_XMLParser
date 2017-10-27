//
// Created by Zhongqi.Shao on 2017/10/23.
//
#pragma once

#include "../TinyXml/tinyxml2.h"
#include "Config.h"
#include <map>
#include <vector>

using namespace tinyxml2;


class NibiruXmlParser {

private:

public:
    int loadXml();

    std::map<std::string, Config> parseConfigXml(const std::string filePath);

};
