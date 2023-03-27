#ifndef USERMODEL_H
#define USERMODEL_H

/*
UserModel类，封装对User表的所有操作
*/

#include "user.hpp"

class UserModel {
public:
    // 构造函数
    UserModel();

    // 析构函数
    ~UserModel();

    // 增
    bool insert(User& user);

    // 删
    
    // 改
    bool updateState(User& user);

    // 查
    User query(int id);

    // 重置User表（服务器异常退出后）
    bool resetState();

private:
    // 
};

#endif