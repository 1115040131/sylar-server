#include <yaml-cpp/yaml.h>

#include "../src/config.h"
#include "../src/log.h"

sylar::ConfigVar<int>::ptr g_int_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");

sylar::ConfigVar<float>::ptr g_float_value_config =
    sylar::Config::Lookup("system.port", (float)10.2f, "system value");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                                         << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                                         << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto& item : node) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << item.first << " - " << item.second.Type() << " - " << level;
            print_yaml(item.second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << std::string(level * 4, ' ')
                                             << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/pyc/dev/sylar/bin/conf/log.yml");
    print_yaml(root, 0);

    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root;
}

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->toString();

    test_yaml();
    return 0;
}