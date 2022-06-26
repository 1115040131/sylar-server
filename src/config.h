//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// config.h
//
// Identification: src/config.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

namespace sylar {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    ConfigVarBase(const std::string& name, const std::string& description = "")
        : m_name(name), m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;

protected:
    std::string m_name;
    std::string m_description;
};

template <class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    std::string toString() override {
        try {
            return boost::lexical_cast<std::string>(m_val);
        } catch (const std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception " << e.what()
                                              << " convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try {
            m_val = boost::lexical_cast<T>(val);
        } catch (const std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception " << e.what()
                                              << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& val) { m_val = val; }

private:
    T m_val;
};

/**
 * @brief ConfigVar的管理类
 * @details 提供便捷的方法创建/访问ConfigVar
 */
class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    /**
     * @brief 获取/创建对应参数名的配置参数
     * @param[in] name 配置参数名称
     * @param[in] default_value 参数默认值
     * @param[in] description 参数描述
     * @details 获取参数名为name的配置参数,如果存在直接返回
     *          如果不存在,创建参数配置并用default_value赋值
     * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
     * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
     */
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                                             const T& default_value, const std::string& description = "") {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " invalid";
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v = std::make_shared<ConfigVar<T>>(name, default_value, description);
        s_datas[name] = v;
        return v;
    }

    /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto iter = s_datas.find(name);
        if (iter == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);

private:
    static ConfigVarMap s_datas;
};

}  // namespace sylar