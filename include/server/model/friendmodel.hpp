#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

/*
封装对friend表的操作
*/

#include "user.hpp"

#include <vector>

class FriendModel {
public:
    // 构造函数
    FriendModel() = default;

    // 析构函数
    ~FriendModel() = default;

    // 添加好友关系
    void addFriend(int userId, int friendId);

    // 返回用户好友列表
    std::vector<User> retFriendLists(int userId);

private:
};

#endif