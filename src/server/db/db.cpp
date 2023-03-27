#include "db.hpp"

#include <string>
#include <muduo/base/Logging.h>

using namespace muduo;

// 数据库的配置信息
static std::string server = "127.0.0.1";
static std::string dbname = "chat";
static std::string user = "root";
static std::string password = "12";

// 构造函数
MySQL::MySQL() {
    // 调用mysql_init函数给MySQL连接开辟了一块资源空间
    conn_ = mysql_init(nullptr);
}

// 析构函数
MySQL::~MySQL() {
    // 如果数据库连接资源指针不为空
    // 则调用mysql_close函数将MySQL连接的资源空间释放掉
    if (conn_ != nullptr) {
        mysql_close(conn_);
    }
}

// 连接数据库（connect函数）
bool MySQL::connect() {
    // 调用mysql_real_connect函数连接MySQL数据库，返回MySQL对象指针
    MYSQL *p = mysql_real_connect(conn_, server.c_str(), user.c_str(), 
                                password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    // 如果连接成功，则调用mysql_query函数写入”set names gbk”
    if (p != nullptr) {
        mysql_query(conn_, "set names gbk");
    }
    return p;
}

// update更新操作
bool MySQL::update(std::string sql) {
    // 调用mysql_query函数对数据库进行更新操作
    if (mysql_query(conn_, sql.c_str())) {
        LOG_ERROR << __FILE__ << " : " << __LINE__ << " : " << sql << "更新失败";
        return false;
    }
    return true;
}

// query查询操作
MYSQL_RES* MySQL::query(std::string sql) {
    // 调用mysql_query函数对数据库进行查询操作
    if (mysql_query(conn_, sql.c_str())) {
        LOG_ERROR << __FILE__ << " : " << __LINE__ << " : " << sql << "查询失败";
        return nullptr;
    }
    return mysql_use_result(conn_);
}