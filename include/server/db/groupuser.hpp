#ifndef GROUPUSER_H
#define GROUPUSER_H

/*
存储群信息
*/

#include "user.hpp"

#include <string>

class GroupUser : public User {
public:
    // 获取
    std::string getGroupRole() { return groupRole_; }

    // 设置
    void setGroupRole(std::string groupRole) { groupRole_ = groupRole; }

private:
    // 表中的数据
    std::string groupRole_;
};

#endif