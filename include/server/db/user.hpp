#ifndef USER_H
#define USER_H

/*
User类，对应User表中的键
*/

#include <string>

class User {
public:
    // 构造函数
    // 初始化各成员变量（给数据库中的字段赋默认值）
    User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
        : id_(id)
        , name_(name)
        , password_(password)
        , state_(state)
    {}

    // 析构函数
    ~User() = default;

    // 清空User的函数
    void clearUser() {
        id_ = -1;
        name_ = "";
        password_ = "";
        state_ = "offline";
    }
    
    // 设置各成员变量的方法（setxxx）
    void setId(int id) { id_ = id; }
    void setName(std::string name) { name_ = name; }
    void setPassword(std::string password) { password_ = password; }
    void setState(std::string state) { state_ = state; }

    // 获取各成员变量的方法（getxxx）
    int getId() { return id_; }
    std::string getName() { return name_; }
    std::string getPassword() { return password_; }
    std::string getState() { return state_; }

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};

#endif

