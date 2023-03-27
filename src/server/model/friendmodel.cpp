#include "friendmodel.hpp"
#include "db.hpp"

#include <iostream>

// 添加好友关系
void FriendModel::addFriend(int userId, int friendId) {
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userId, friendId);

    // 若连接MySQL成功，则执行sql语句
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
        
        // 互相添加好友
        sprintf(sql, "insert into friend values(%d, %d)", friendId, userId);
        mysql.update(sql);
    }
}

// 返回用户好友列表
std::vector<User> FriendModel::retFriendLists(int userId) {
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on b.friendid = a.id where b.userid = %d", userId);
    
    // 如果连接MySQL成功（调用MySQL::connect函数），则调用MySQL::query查询数据库，获取返回值（MYSQL_RES* res）
    MySQL mysql;
    std::vector<User> friendLists;
    if (mysql.connect()) {
        MYSQL_RES* res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                // std::cout << "friendid : " << atoi(row[0]) << std::endl;
                user.setName(row[1]);
                user.setState(row[2]);
                friendLists.push_back(user);
            }
            // 调用mysql_free_result函数释放资源
            mysql_free_result(res);
        }
    }
    // 返回所有的离线消息（vector<User>）
    return friendLists;
}