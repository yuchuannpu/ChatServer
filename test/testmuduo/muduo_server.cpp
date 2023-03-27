/*
基于muduo网络库开发的服务器程序
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

class ChatServer {
public:
    // ChatServer的构造函数
    ChatServer(
        EventLoop* _loop, // 事件循环
        const InetAddress& listenAddr, // 绑定ip地址和端口号
        const string& nameArg // 服务器的名字
        // Option option = kNoReusePort // tcp协议选项
    )
        : _server(_loop, listenAddr, nameArg)
        , _loop(loop) {
        // 注册用户连接的创建和断开的回调函数
        _server.setConnectionCallback(std::bind(&ChatServer::connectionCallback, this, std::placeholders::_1));

        // 注册用户连接触发读写事件的回调函数
        _server.setMessageCallback(std::bind(&ChatServer::messageCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        // 设置服务端的线程数量
        _server.setThreadNum(4); // 一个io线程，3个工作线程
    }

    // 开启事件循环
    void start() { _server.start(); }

    // 用户连接的创建和断开的回调函数
    void connectionCallback(const TcpConnectionPtr& conn) {
        if (conn->connected()) { // 用户创建连接
            // 打印对端和本端的ip地址和端口号
            std::cout << "peer address : " << conn->peerAddress().toIpPort() << std::endl;
            std::cout << "local address : " << conn->localAddress().toIpPort() << std::endl;
        }
        else { // 用户断开连接
            // 输出用户下线
            std::cout << "offline" << std::endl;
            conn->shutdown(); // close(fd)
            // _loop->quit();
        }
    }

    // 用户连接触发读写事件的回调函数
    void messageCallback(const TcpConnectionPtr& conn, // 连接
                            Buffer *buffer,  // buffer缓冲区
                            Timestamp time) { // 时间
        // 将用户传来的数据原封不动地回传回去
        string buf = buffer->retrieveAllString();
        std::cout << "recv data : " << buf << std::endl;
        std::cout << "time : " << time.toString() << std::endl;
        conn->send(buf);
    }

    // 启动事件循环
    void start() { _server.start(); }
    
private:
    // 创建TcpServer对象
    muduo::net::TcpServer _server;
    // 创建EventLoop指针
    muduo::EventLoop *_loop;

};


int main() {
    // 创建epoll对象
    EventLoop* epollLoop;
    // 设置ip地址和端口
    InetAddress addr("127.0.0.1", 6000); // 监听在6000端口，服务器的ip地址为127.0.0.1
    // 封装ChatServer
    ChatServer server(&epollLoop, addr, "ChatServer");

    // 将接收到的listenfd调用epoll_ctl添加到epoll中
    server.start();
    // 启动事件循环：开启epoll_wait
    epollLoop.loop();

    return 0;
}
