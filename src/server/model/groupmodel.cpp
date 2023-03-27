#include "groupmodel.hpp"
#include "db.hpp"

// 创建群
bool GroupModel::createGroup(Group& group) {
    // 传入groupname和groupdesc，填入AllGroup中，自动生成groupid
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')", 
            group.getGroupName().c_str(), group.getGroupDesc().c_str());

    // 执行sql语句
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群
bool GroupModel::joinGroup(int userId, int groupId, std::string groupRole) {
    // 在GroupUser表中添加数据
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupId, userId, groupRole.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

// 查询用户所在群组信息，用于群聊
std::vector<Group> GroupModel::queryGroupInfo(int userId) {
    // 在AllGroup表和GroupUser表中，根据userid查询用户所在的所有群的群组id
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a \
            inner join groupuser b on a.id = b.groupid where b.userid = %d", userId);

    MySQL mysql;
    std::vector<Group> groupInfo;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                groupInfo.push_back(group);
            }
            mysql_free_result(res);
        }
        else {
            // 如果查询失败
        }
    }
    else {
        // 如果连接MySQL失败
    }

    // 在GroupUser表和User表中，根据群组id查询群成员信息
    for (Group& group : groupInfo) {
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a \
                inner join groupuser b on b.userid = a.id where b.groupid = %d", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                GroupUser groupUser;
                groupUser.setId(atoi(row[0]));
                groupUser.setName(row[1]);
                groupUser.setState(row[2]);
                groupUser.setGroupRole(row[3]);
                group.getGroupUser().push_back(groupUser);
            }
            mysql_free_result(res);
        }
        else {
            // 如果查询失败
        }
    }
    return groupInfo;
}

// 根据群id查询其他群成员的id，用于群聊
std::vector<int> GroupModel::queryUserInfo(int userId, int groupId) {
    // 在groupuser表中，根据groupid查询该群组中除了我的userid以外的其他人的userid
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupId, userId);

    std::vector<int> userInfo;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr) {
                userInfo.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
        else {
            // 如果查询失败
        }
    }
    return userInfo;
}