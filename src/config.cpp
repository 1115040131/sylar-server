//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// config.cpp
//
// Identification: src/config.cpp
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#include "config.h"

#include <list>

namespace sylar {

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    auto iter = s_datas.find(name);
    return iter == s_datas.end() ? nullptr : iter->second;
}

Config::ConfigVarMap Config::s_datas;

static void ListAllMember(const std::string& prefix,
                          const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node>>& output) {
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
        return;
    }
    output.emplace_back(prefix, node);
    if (node.IsMap()) {
        for (auto& item : node) {
            ListAllMember(prefix.empty() ? item.first.Scalar() : prefix + "." + item.first.Scalar(), item.second, output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> nodes;
    ListAllMember("", root, nodes);

    for (auto& node : nodes) {
        std::string key = node.first;
        if (key.empty()) {
            continue;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = LookupBase(key);

        if (var) {
            if (node.second.IsScalar()) {
                var->fromString(node.second.Scalar());
            } else {
                std::stringstream ss;
                ss << node.second;
                var->fromString(ss.str());
            }
        }
    }
}

}  // namespace sylar