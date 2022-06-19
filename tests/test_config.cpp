#include "../src/config.h"
#include "../src/log.h"

sylar::ConfigVar<int>::ptr g_ini_value_config =
    sylar::Config::Lookup("system.port", (int)8080, "system port");

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_ini_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_ini_value_config->getName();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_ini_value_config->getDescription();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_ini_value_config->toString();
    return 0;
}