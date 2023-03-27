#include "usermodel.hpp"
#include "db.hpp"

// 构造函数
UserModel::UserModel() {}

// 析构函数
UserModel::~UserModel() {}

// 增
bool UserModel::insert(User& user) {
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')", 
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    
    // 如果连接MySQL成功（调用MySQL::connect函数），则调用MySQL::update更新数据库
    MySQL mysql;
    if (mysql.connect()) {
        // 若更新成功，则获取数据库给该条数据生成的主键id，存入User::id中
        // 用户从User类中获取即可
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 删

// 改
bool UserModel::updateState(User& user) {
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

    // 如果连接MySQL成功（调用MySQL::connect函数），则调用MySQL::update更新数据库
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

// 查
User UserModel::query(int id) {
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);

    // 如果连接MySQL成功（调用MySQL::connect函数），则调用MySQL::query查询数据库，获取返回值（MYSQL_RES* res）
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        // 若查询成功（返回值不为空），则
        if (res != nullptr) {
            // 调用mysql_fetch_row函数获取结果的一行
            MYSQL_ROW row = mysql_fetch_row(res);
            // 如果有数据，则
            if (row != nullptr) {
                // 设置User对象：将获取到的用户信息存入User对象
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                // 调用mysql_free_result函数释放资源（释放MYSQL_RES* res）
                mysql_free_result(res);
                return user;
            }
        }
    }
    
    // 如果出错了，则返回一个默认的User对象
    return User();
    // User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
}

// 重置User表（服务器异常退出后）
bool UserModel::resetState() {
    // 组装sql语句
    char sql[1024] = "update user set state = 'offline'";

    // 若连接MySQL成功，则执行sql语句
    MySQL mysql;
    if (mysql.connect()) {
        mysql.query(sql);
        return true;
    }
    return false;
}