#include "offlinemessagemodel.hpp"
#include "db.hpp"

#include <vector>

// 插入用户的离线消息
void OfflineMessageModel::insert(int userid, std::string msg) {
    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());

    // 若连接成功MySQL，则更新MySQL，否则不做任何操作
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

// 删除用户的离线消息
void OfflineMessageModel::remove(int userid) {
    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = %d", userid);

    // 若连接成功MySQL，则更新MySQL，否则不做任何操作
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int userid) {
    // 组装sql
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    // 如果连接MySQL成功（调用MySQL::connect函数），则调用MySQL::query查询数据库，获取返回值（MYSQL_RES* res）
    MySQL mysql;
    std::vector<std::string> vec;
    if (mysql.connect()) {
        MYSQL_RES* res =  mysql.query(sql);
        // 若查询成功（返回值不为空），则：
        // 把userid用户所有的离线消息放入vector<string>中返回
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }

            // 调用mysql_free_result函数释放资源（释放MYSQL_RES* res）
            mysql_free_result(res);
        }
    }
    return vec;
}