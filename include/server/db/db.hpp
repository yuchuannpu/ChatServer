#ifndef DB_H
#define DB_H

/*
数据库类
*/

#include <mysql/mysql.h>
#include <string>

class MySQL {
public:
    // 构造函数
    MySQL();

    // 析构函数
    ~MySQL();

    // 连接数据库（connect函数）
    bool connect();

    // update更新操作
    bool update(std::string sql);

    // query查询操作
    MYSQL_RES* query(std::string sql);

    // 获取MySQL连接
    MYSQL* getConnection() { return conn_; }

private:
    // MYSQL连接
    MYSQL* conn_;
};

#endif