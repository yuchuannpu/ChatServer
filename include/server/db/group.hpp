#ifndef GROUP_H
#define GROUP_H

/*
存储群信息
*/

#include "groupuser.hpp"

#include <string>
#include <vector>

class Group {
public:
    // 构造函数  
    Group(int id = -1, std::string groupName = "", std::string groupDesc = "")
        : id_(id)
        , groupName_(groupName)
        , groupDesc_(groupDesc)
    {}

    // 析构函数

    // 获取
    int getId() { return id_; }
    std::string getGroupName() { return groupName_; }
    std::string getGroupDesc() { return groupDesc_; }
    std::vector<GroupUser>& getGroupUser() { return groupUser_; }

    // 设置
    void setId(int id) { id_ = id; }
    void setGroupName(std::string groupName) { groupName_ = groupName; }
    void setGroupDesc(std::string groupDesc) { groupDesc_ = groupDesc; }

private:
    // 表中的数据
    int id_;
    std::string groupName_;
    std::string groupDesc_;

    // 群成员信息
    std::vector<GroupUser> groupUser_; 
};

#endif