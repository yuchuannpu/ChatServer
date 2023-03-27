#ifndef GROUPMODEL_H
#define GROUPMODEL_H

/*
封装对AllGroup表和GroupUser表的操作
*/

#include "group.hpp"

#include <string>

class GroupModel {
public:
    // 构造函数
    GroupModel() = default;
    
    // 析构函数
    ~GroupModel() = default;

    // 创建群
    bool createGroup(Group& group);

    // 加入群
    bool joinGroup(int userId, int groupId, std::string groupRole = "normal");

    // 查询用户所在群组信息
    std::vector<Group> queryGroupInfo(int userId);

    // 根据群id查询其他群成员的id，用于群聊
    std::vector<int> queryUserInfo(int userId, int groupId);

private:
};

#endif