#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

/*
对offlinemessage表操作的类
*/

#include <string>
#include <vector>

class OfflineMessageModel {
public:
    // 构造函数
    OfflineMessageModel() = default;

    // 析构函数
    ~OfflineMessageModel() = default;

    // 插入用户的离线消息
    void insert(int userid, std::string msg);

    // 删除用户的离线消息
    void remove(int userid);

    // 查询用户的离线消息
    std::vector<std::string> query(int userid);
};

#endif