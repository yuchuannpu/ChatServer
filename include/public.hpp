#ifndef PUBLIC_H
#define PUBLIC_H

/*
server和client的公共文件
*/

#include <string>

enum MsgType {
    LOGIN_MSG = 1, // 登录业务
    LOGIN_MSG_ACK, // 登录业务响应

    REG_MSG, // 注册业务
    REG_MSG_ACK, // 注册业务响应

    ONE_CHAT_MSG, // 单聊业务
    GROUP_CHAT_MSG, // 群聊业务

    ADD_FRIEND_MSG, // 添加好友业务
    ADD_FRIEND_MSG_ACK,

    // 群聊的相关业务
    CREATE_GROUP_MSG, // 创建群聊业务
    CREATE_GROUP_MSG_ACK,
    JOIN_GROUP_MSG, // 加入群聊业务
    JOIN_GROUP_MSG_ACK,

    LOGOFF_MSG, // 用户注销业务
    LOGOFF_MSG_ACK
};

const std::string CREATOR = "creator";
const std::string MEMBER = "normal";

#endif