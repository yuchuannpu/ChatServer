#include "chatserver.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <signal.h>

void resetHandler(int) {
    // 调用服务器异常退出的业务处理函数ChatService::srvUnexpectedExit
    ChatService::instance()->srvUnexpectedExit();

    // 终止程序
    exit(0);
}

int main(int argc, char** argv) {
    if (argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    signal(SIGINT, resetHandler);

    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 初始化server对象
    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "chatserver");

    // 启动服务器
    server.start();
    loop.loop();
    
    return 0;
}