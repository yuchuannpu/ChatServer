#ifndef CHATSERVICE_H
#define CHATSERVICE_H

/*
业务类
*/

#include "chatserver.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

#include <functional>
#include <unordered_map>
#include <mutex>

using json = nlohmann::json;

// 定义消息id的业务处理函数
using MsgHandler = std::function<void(const TcpConnectionPtr&, json&, Timestamp)>;

class ChatService {
public:
    // 构造函数
    ChatService();

    // 析构函数
    ~ChatService() = default;

    // 获取单例对象的接口函数
    static ChatService* instance();

    // 登录的业务处理函数
    void login(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 注册的业务处理函数
    void reg(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 获取消息id对应的业务处理函数
    MsgHandler getHandler(int msgid);

    // 用户异常退出的回调函数
    void userUnexpectedExit(const TcpConnectionPtr& conn);

    // 单聊的业务处理函数
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 服务器异常退出的业务处理函数
    void srvUnexpectedExit();

    // 添加好友的业务处理函数
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 创建群聊的业务处理函数
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    
    // 加入群组的业务处理函数
    void joinGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 群组聊天的业务处理函数
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 注销的业务处理函数
    void logOff(const TcpConnectionPtr& conn, json& js, Timestamp time);

    // 监听到channel中有数据到达时所调用的回调函数
    void getChannelMsgFunc(int userId, std::string msg);

private:
    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int, MsgHandler> msgHandlerMap_;

    // UserModel类对象，用于操作User表
    UserModel userModel_;

    // map: 用户id <-> TcpConnectionPtr
    std::unordered_map<int, TcpConnectionPtr> connMap_;
    // 对map加锁
    std::mutex connMapMutex_;

    // OfflineMessageModel类对象，用于操作OfflineMessageModel表
    OfflineMessageModel offlineMessageModel_;

    // 操作friend表的friendModel类对象
    FriendModel friendModel_;

    // 操作allgroup表和groupuser表的类对象
    GroupModel groupModel_;

    // Redis类对象，用于操作redis
    Redis redis_;
};

#endif