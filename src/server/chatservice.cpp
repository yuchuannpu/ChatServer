/*
业务类
*/

#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"
#include "offlinemessagemodel.hpp"
#include "groupmodel.hpp"

#include <muduo/base/Logging.h>
#include <iostream>

using namespace muduo;
using json = nlohmann::json;

// 构造函数
ChatService::ChatService() {
    // 初始化各成员变量：在map中注册消息id和对应的业务处理函数
    msgHandlerMap_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, 
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({REG_MSG, std::bind(&ChatService::reg, this, 
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({JOIN_GROUP_MSG, std::bind(&ChatService::joinGroup, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    msgHandlerMap_.insert({LOGOFF_MSG, std::bind(&ChatService::logOff, this,
                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)});
    
    // 连接redis-server
    // 如果连接成功，则设置当监听的channel中有消息到达时要调用的回调函数
    if (redis_.connectRedis()) {
        redis_.setChannelMsgHandler(std::bind(&ChatService::getChannelMsgFunc, this, std::placeholders::_1, std::placeholders::_2));
    }
}

// 获取消息id对应的业务处理函数
MsgHandler ChatService::getHandler(int msgid) {
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end()) {
        return [=](const TcpConnectionPtr& conn, json& js, Timestamp time) {
            LOG_ERROR << "msgid : " << msgid << " Handler is not find";
        };
    }
    else {
        return msgHandlerMap_[msgid];
    }
}

// 获取单例对象的接口函数
ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

// 登录的业务处理函数
void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    LOG_INFO << "do login service";

    // 获取客户端传来的用户的主键id和密码
    int id = js["id"].get<int>();
    std::string passsword = js["password"];

    // 根据主键id从数据库中获取对应的User对象（调用UserModel::query函数）
    User user = userModel_.query(id);

    // 根据主键id获取密码，进行比较
    // 如果相等表示登录成功，则：
    if (user.getPassword() == passsword) {
        // 判断用户是否重复登录（User的state是否已经为”online”）
        // 如果用户已经登录，则封装errmsg为“该账号已登录，请重新输入账号”
        if (user.getState() == "online") {
            // 封装json串
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户已经登录，请勿重复登录";
            // 调用send发送给用户
            conn->send(response.dump());
        }
        else { // 如果登录成功，则：
            // 更新map
            {
                std::lock_guard<std::mutex> lock(connMapMutex_);
                connMap_.insert({id, conn});
            }

            // 在redis中订阅以userid命名的channel
            redis_.subscribeChannel(id);

            // 更新用户状态信息：将User表中的state从offline变为online
            user.setState("online");
            userModel_.updateState(user);

            // 封装json串
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["userid"] = user.getId();
            response["username"] = user.getName();
            response["userstate"] = user.getState();
            
            // 处理离线消息：
            // 查询该用户是否有离线消息
            std::vector<std::string> offlineMessage = offlineMessageModel_.query(id);
            // 如果有，则：
            if (!offlineMessage.empty()) {
                // 将离线消息封装进json串中回复给客户端
                response["offlinemsg"] = offlineMessage;
                // 将offlineMessage表中该用户的离线消息删除
                offlineMessageModel_.remove(id);
            }

            // 获取用户好友列表
            std::vector<User> friendLists = friendModel_.retFriendLists(id);
            if (!friendLists.empty()) {
                std::vector<std::string> friends;
                for (User& friendList : friendLists) {
                    json js;
                    js["friendid"] = friendList.getId();
                    js["friendname"] = friendList.getName();
                    js["friendstate"] = friendList.getState();
                    friends.push_back(js.dump());
                }
                response["friends"] = friends;
            }

            // 获取用户的群组信息
            std::vector<Group> groupLists = groupModel_.queryGroupInfo(id);
            if (!groupLists.empty()) {
                std::vector<std::string> group;
                for (Group& groupList : groupLists) {
                    json js;
                    js["groupid"] = groupList.getId();
                    js["groupname"] = groupList.getGroupName();
                    js["groupdesc"] = groupList.getGroupDesc();

                    // 群成员信息
                    std::vector<std::string> userString;
                    for (GroupUser& groupUser : groupList.getGroupUser()) {
                        json userJs;
                        userJs["gmemid"] = groupUser.getId();
                        userJs["gmemname"] = groupUser.getName();
                        userJs["gmemstate"] = groupUser.getState();
                        userJs["gmemrole"] = groupUser.getGroupRole();
                        userString.push_back(userJs.dump());
                    }
                    js["groupusers"] = userString;
                    group.push_back(js.dump());
                }
                response["groups"] = group;
            }
            
            // 调用send发送给用户
            conn->send(response.dump());
        }
    }
    else {
        // 封装json串
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "username or password error";
        // 调用send发送给用户
        conn->send(response.dump());
    }
}

// 注册的业务处理函数
void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    LOG_INFO << "do reg service";

    // 获取客户端传来的name和password
    std::string name = js["name"];
    std::string password = js["password"];

    // 设置User类对象
    User user;
    user.setName(name);
    user.setPassword(password);

    // std::cout << "password : " << password << std::endl;

    // 操作User数据库：调用UserModel的insert函数
    // 如果插入成功（注册成功），则：
    if (userModel_.insert(user)) {
        // 封装json串
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        // response["id"] = user.getId();
        // 调用send发送给用户
        conn->send(response.dump());
    }
    // 如果插入失败（注册失败），则：
    else {
        // 封装json串
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "reg >> reg error";
        // 调用send发送给用户
        conn->send(response.dump());
    }
}

// 用户异常退出的回调函数
void ChatService::userUnexpectedExit(const TcpConnectionPtr& conn) {
    // 将该用户从map（<用户id, TcpConnectionPtr>）中删除
    User user;
    {
        std::lock_guard<std::mutex> lock(connMapMutex_);
        for (auto it = connMap_.begin(); it != connMap_.end(); it++) {
            if (it->second == conn) {
                user.setId(it->first);
                connMap_.erase(it);
                break;
            }
        }
    }

    // 如果有该用户
    if (user.getId() != -1) {
        // 更新用户的状态信息
        user.setState("offline");
        userModel_.updateState(user);
    }
}

// 单聊的业务处理函数
void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    int toid = js["toid"].get<int>();
    {
        std::lock_guard<std::mutex> lock(connMapMutex_);
        // 获取接收方id对应的TcpConnectionPtr
        auto it = connMap_.find(toid);
        if (it != connMap_.end()) {
            // 如果接收方在线，则调用send函数转发消息给接收方
            it->second->send(js.dump());
            return;
        }
    }

    // 查询接收方的登录状态
    // 可能是不在一个服务器上登录
    User user;
    if (user.getState() == "online") {
        // 若在线则说明接收方不在该服务器上登录
        // 则向redis中将消息publish到以接收方userid命名的channel上
        redis_.pubMegToChannel(toid, js.dump());
    }
    else {
        // 如果接收方不在线，则存储离线消息
        offlineMessageModel_.insert(toid, js.dump());
    }
}

// 服务器异常退出的业务处理函数
void ChatService::srvUnexpectedExit() {
    // 调用userModel的接口将User表中所有用户的状态从online设置为offline
    userModel_.resetState();
}

// 添加好友的业务处理函数
void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    // 调用friendModel的添加好友操作
    int userId = js["userid"].get<int>();
    int friendId = js["friendid"].get<int>();
    friendModel_.addFriend(userId, friendId);

    json response;
    response["msgid"] = ADD_FRIEND_MSG_ACK;
    conn->send(response.dump());
}

// 创建群聊的业务处理函数
void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    // 创建群组
    std::string groupName = js["gourpname"];
    std::string groupDesc = js["groupdesc"];
    Group group(-1, groupName, groupDesc);

    if (groupModel_.createGroup(group)) {
        // 如果创建成功则将用户加入到群组中
        int userId = js["userid"].get<int>();
        groupModel_.joinGroup(userId, group.getId(), CREATOR);
    }

    json response;
    response["msgid"] = CREATE_GROUP_MSG_ACK;
    conn->send(response.dump());
}

// 加入群组的业务处理函数
void ChatService::joinGroup(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    // 将用户加入到群组中
    int userId = js["userid"].get<int>();
    int groupId = js["groupid"].get<int>();
    groupModel_.joinGroup(userId, groupId, MEMBER);

    json response;
    response["msgid"] = JOIN_GROUP_MSG_ACK;
    conn->send(response.dump());
}

// 群组聊天的业务处理函数
void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    // 查询该用户所在群组的其他用户的id
    int fromId = js["fromid"].get<int>();
    int groupId = js["groupid"].get<int>();
    std::vector<int> memberId = groupModel_.queryUserInfo(fromId, groupId);

    // 向用户发送群聊消息，若用户在线则直接转发消息，若用户离线则存储离线消息
    std::lock_guard<std::mutex> lock(connMapMutex_);
    for (int id : memberId) {
        auto it = connMap_.find(id);
        // 用户在线
        if (it != connMap_.end()) {
            it->second->send(js.dump());
        }
        // 用户不在线
        else {
            // 查询接收方的登录状态
            User user;
            // 可能是不在一个服务器上登录
            if (user.getState() == "online") {
                // 若在线则说明接收方不在该服务器上登录
                // 则向redis中将消息publish到以接收方userid命名的channel上
                redis_.pubMegToChannel(fromId, js.dump());
            }
            else {
                // 存储离线消息
                offlineMessageModel_.insert(fromId, js.dump());
            }
        }
    }
}

// 注销的业务处理函数
void ChatService::logOff(const TcpConnectionPtr& conn, json& js, Timestamp time) {
    // 将该用户从map（<用户id, TcpConnectionPtr>）中删除
    User user;
    {
        std::lock_guard<std::mutex> lock(connMapMutex_);
        for (auto it = connMap_.begin(); it != connMap_.end(); it++) {
            if (it->second == conn) {
                user.setId(it->first);
                connMap_.erase(it);
                break;
            }
        }
    }

    // 如果有该用户
    int userId = user.getId();
    if (userId != -1) {
        // 更新用户的状态信息
        user.setState("offline");
        userModel_.updateState(user);
    }

    // 取消订阅redis中以userid命名的channel
    redis_.unsubscribeChannel(userId);
    
    json response;
    response["msgid"] = LOGOFF_MSG_ACK;
    conn->send(response.dump());
}

// 监听到channel中有数据到达时所调用的回调函数
void ChatService::getChannelMsgFunc(int userId, std::string msg) {
    std::lock_guard<std::mutex> lock(connMapMutex_);
    // 在connMap中查询是否有该userid的连接（TcpConnectionPtr）
    auto it = connMap_.find(userId);
    // 如果有该用户，则转发
    if (it != connMap_.end()) {
        it->second->send(msg);
    }
    else {
        // 如果该用户在redis channel转发消息的过程中下线了，则存储离线数据
        offlineMessageModel_.insert(userId, msg);
    }
}