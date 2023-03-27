#ifndef CHATSERVER_H
#define CHATSERVER_H

/*
服务器类
*/

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>
#include <string>

using namespace muduo;
using namespace muduo::net;

class ChatServer {
public:
    // 构造函数
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg);

    // 析构函数
    ~ChatServer() = default;

    // 启动服务的函数
    void start();

private:
    // 触发两类事件的回调函数
    void onConenction(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);

    // TcpServer对象
    TcpServer server_;

    // EventLoop指针
    EventLoop* loop_;
};

#endif