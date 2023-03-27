#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <string>
#include <iostream>

using json = nlohmann::json;

// 构造函数
ChatServer::ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg)
    : loop_(loop)
    , server_(loop, listenAddr, nameArg)
{
    // 设置两个回调函数
    server_.setConnectionCallback(std::bind(&ChatServer::onConenction, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, 
                            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    // 设置线程数量
    server_.setThreadNum(3);
}

// 启动服务的函数
void ChatServer::start() {
    // 调用TcpServer::start函数
    server_.start();
}

// 触发两类事件的回调函数
void ChatServer::onConenction(const TcpConnectionPtr& conn) {
    // 如果客户端断开连接
    if (!conn->connected()) {
        ChatService::instance()->userUnexpectedExit(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) {
    // 将数据从Buffer缓冲区中拿取出来
    std::string buf = buffer->retrieveAllAsString();

    // 反序列化数据
    json js = json::parse(buf);

    // 取出消息id（消息类型）等进行相应的业务逻辑处理
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}