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
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

// 基础类型转换
// F from_type, T to_type
template <class FromType, class ToType>
class LexicalCast {
public:
    ToType operator()(const FromType& v) {
        return boost::lexical_cast<ToType>(v);
    }
};

// vector支持
template <class ValueType>
class LexicalCast<std::string, std::vector<ValueType>> {
public:
    std::vector<ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<ValueType> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.emplace_back(LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::vector<ValueType>, std::string> {
public:
    std::string operator()(const std::vector<ValueType>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<ValueType, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// list支持
template <class ValueType>
class LexicalCast<std::string, std::list<ValueType>> {
public:
    std::list<ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<ValueType> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.emplace_back(LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::list<ValueType>, std::string> {
public:
    std::string operator()(const std::list<ValueType>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<ValueType, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// set支持
template <class ValueType>
class LexicalCast<std::string, std::set<ValueType>> {
public:
    std::set<ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<ValueType> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::set<ValueType>, std::string> {
public:
    std::string operator()(const std::set<ValueType>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<ValueType, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_set支持
template <class ValueType>
class LexicalCast<std::string, std::unordered_set<ValueType>> {
public:
    std::unordered_set<ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<ValueType> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::unordered_set<ValueType>, std::string> {
public:
    std::string operator()(const std::unordered_set<ValueType>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<ValueType, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// map支持
template <class ValueType>
class LexicalCast<std::string, std::map<std::string, ValueType>> {
public:
    std::map<std::string, ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, ValueType> vec;
        std::stringstream ss;
        for (auto iter = node.begin(); iter != node.end(); ++iter) {
            ss.str("");
            ss << iter->second;
            vec.emplace(iter->first.Scalar(), LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::map<std::string, ValueType>, std::string> {
public:
    std::string operator()(const std::map<std::string, ValueType>& v) {
        YAML::Node node;
        for (auto& [key, value] : v) {
            node[key] = YAML::Load(LexicalCast<ValueType, std::string>()(value));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_map支持
template <class ValueType>
class LexicalCast<std::string, std::unordered_map<std::string, ValueType>> {
public:
    std::unordered_map<std::string, ValueType> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, ValueType> vec;
        std::stringstream ss;
        for (auto iter = node.begin(); iter != node.end(); ++iter) {
            ss.str("");
            ss << iter->second;
            vec.emplace(iter->first.Scalar(), LexicalCast<std::string, ValueType>()(ss.str()));
        }
        return vec;
    }
};

template <class ValueType>
class LexicalCast<std::unordered_map<std::string, ValueType>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, ValueType>& v) {
        YAML::Node node;
        for (auto& [key, value] : v) {
            node[key] = YAML::Load(LexicalCast<ValueType, std::string>()(value));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 配置参数模板子类,保存对应类型的参数值
 * @details T 参数的具体类型
 *          FromStr 从std::string转换成T类型的仿函数
 *          ToStr 从T转换成std::string的仿函数
 *          std::string 为YAML格式的字符串
 */
template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    // 配置更改回调函数
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    std::string toString() override {
        try {
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        } catch (const std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception " << e.what()
                                              << " convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (const std::exception& e) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception " << e.what()
                                              << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    std::string getTypeName() const override { return typeid(T).name(); }

    const T getValue() const { return m_val; }
    void setValue(const T& val) {
        if (val == m_val) {
            return;
        }
        for (auto& i : m_cbs) {
            i.second(m_val, val);
        }
        m_val = val;
    }

    void addListener(uint64_t key, on_change_cb cb) {
        m_cbs[key] = cb;
    }

    void delListener(uint64_t key) {
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) {
        auto iter = m_cbs.find(key);
        return iter == m_cbs.end() ? nullptr : iter->second;
    }

    void clearListener() {
        m_cbs.clear();
    }

private:
    T m_val;
    // 变更回调函数组, uint_64 key, 要求唯一, 可以用hash
    std::map<uint64_t, on_change_cb> m_cbs;
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
        auto iter = GetDatas().find(name);
        if (iter != GetDatas().end()) {
            // 如果同名key对应值类型不同, dynamic_cast转换失败, 返回nullptr
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
            if (tmp) {
                SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists";
                return tmp;
            } else {
                SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " exists but type not "
                                                  << typeid(T).name() << " real type = " << iter->second->getTypeName()
                                                  << " " << iter->second->toString();
                return nullptr;
            }
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name = " << name << " invalid";
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v = std::make_shared<ConfigVar<T>>(name, default_value, description);
        GetDatas()[name] = v;
        return v;
    }

    /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto iter = GetDatas().find(name);
        if (iter == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    static ConfigVarBase::ptr LookupBase(const std::string& name);

private:
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

}  // namespace sylar