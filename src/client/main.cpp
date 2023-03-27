/*
客户端程序，用来测试服务器的功能
*/

#include "json.hpp"
#include "public.hpp"
#include "user.hpp"
#include "group.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <semaphore.h>
#include <unordered_map>
#include <functional>

using json = nlohmann::json;

const int LOGIN = 1;
const int REG = 2;
const int EXIT = 3;

// 用于读写线程之间的通信
sem_t g_rwsem;

// 重新来时要进入主界面，标志位
bool g_isEnterUserInterface = false;

// 标志是否登录成功
bool g_isLoginSuccess = false;

// 记录用户的信息
User g_user;

// 用户的好友信息
std::vector<User> g_friends;

// 用户所在群的群组信息
std::vector<Group> g_groups;

// 系统支持的客户端命令列表
std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"onechat", "一对一聊天，格式onechat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"joingroup", "加入群组，格式joingroup:groupid"},
    {"groupchat", "群聊，格式groupchat:groupid:message"},
    {"logoff", "注销，格式logoff"}
};

void help(int sockfd = 0, std::string str = "");
void oneChat(int sockfd, std::string paramBuf);
void addFriend(int sockfd, std::string paramBuf);
void createGroup(int sockfd, std::string paramBuf);
void joinGroup(int sockfd, std::string paramBuf);
void groupChat(int sockfd, std::string paramBuf);
void logOff(int sockfd, std::string paramBuf = "");

// 注册系统支持的客户端命令处理
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help}, // 帮助
    {"onechat", oneChat}, // 单聊
    {"addfriend", addFriend}, // 添加好友
    {"creategroup", createGroup}, // 建群
    {"joingroup", joinGroup}, // 加群
    {"groupchat", groupChat}, // 群聊
    {"logoff", logOff} // 注销
};

// 获取系统时间
std::string getTime() {
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char data[60] = {0};
    sprintf(data, "%d/%02d/%02d %02d:%02d:%02d", 
        ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return std::string(data);
}

// 业务处理函数
// 打印帮助信息
void help(int sockfd, std::string str) {
    std::cout << "show command list >>> " << std::endl;
    for (auto& command : commandMap) {
        std::cout << command.first << " : " << command.second << std::endl;
    }
}
// 单聊的请求业务
void oneChat(int sockfd, std::string paramBuf) {
    // 从传入的字符串（封装了参数）中取出参数
    int idx = paramBuf.find(":");
    if (-1 == idx) {
        std::cerr << "one chat command invalid!" << std::endl;
        return;
    }
    int toid = atoi(paramBuf.substr(0, idx).c_str());
    std::string msg = paramBuf.substr(idx + 1, paramBuf.size() - idx);

    // 封装json串
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["fromid"] = g_user.getId();
    js["fromname"] = g_user.getName();
    js["time"] = getTime();
    js["toid"] = toid;
    js["msg"] = msg;
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "one chat >> send msg : %s (error)" << jsString << std::endl;
    }
}
// 添加好友的请求业务
void addFriend(int sockfd, std::string paramBuf) {
    // 从传入的字符串（封装了参数）中取出参数
    int friendId = atoi(paramBuf.c_str());

    // 封装json串
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["userid"] = g_user.getId();
    js["friendid"] = friendId;
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "addfriend >> send msg : %s (error)" << jsString << std::endl;
    }
}
// 创建群聊的请求业务
void createGroup(int sockfd, std::string paramBuf) {
    // 从传入的字符串（封装了参数）中取出参数
    int idx = paramBuf.find(":");
    if (-1 == idx) {
        std::cerr << "createGroup >> command invalid!" << std::endl;
        return;
    }
    std::string gourpName = paramBuf.substr(0, idx);
    std::string groupDesc = paramBuf.substr(idx + 1, paramBuf.size() - idx);

    // 封装json串
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["userid"] = g_user.getId();
    js["gourpname"] = gourpName;
    js["groupdesc"] = groupDesc;
    // js["grouprole"] = CREATOR;
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "createGroup >> send msg : %s (error)" << jsString << std::endl;
    }
}
// 加群的请求业务
void joinGroup(int sockfd, std::string paramBuf) {
    // 从传入的字符串（封装了参数）中取出参数
    int groupId = atoi(paramBuf.c_str());

    // 封装json串
    json js;
    js["msgid"] = JOIN_GROUP_MSG;
    js["userid"] = g_user.getId();
    js["groupid"] = groupId;
    // js["grouprole"] = MEMBER;
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "joinGroup >> send msg : %s (error)" << jsString << std::endl;
    }
}
// 群聊
void groupChat(int sockfd, std::string paramBuf) {
    // 从传入的字符串（封装了参数）中取出参数
    int idx = paramBuf.find(":");
    if (-1 == idx) {
        std::cerr << "groupChat >> command invalid!" << std::endl;
        return;
    }
    int groupId = atoi(paramBuf.substr(0, idx).c_str());
    std::string msg = paramBuf.substr(idx + 1, paramBuf.size() - idx);

    // 封装json串
    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["fromid"] = g_user.getId();
    js["fromusername"] = g_user.getName();
    js["groupid"] = groupId;
    js["msg"] = msg;
    js["time"] = getTime();
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "groupChat >> send msg : %s (error)" << jsString << std::endl;
    }
}
// 注销的请求业务函数
void logOff(int sockfd, std::string paramBuf) {
    // 取出参数
    
    // 封装json串
    json js;
    js["msgid"] = LOGOFF_MSG;
    js["userid"] = g_user.getId();
    std::string jsString = js.dump();

    // 发送给服务端
    int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
    if (-1 == len)
    {
        std::cerr << "logOff >> send msg : %s (error)" << jsString << std::endl;
    }
    else {
        g_isEnterUserInterface = false;
    }
}

// 显示用户相关的信息
void showUserInfo() {
    std::cout << "======================login user======================" << std::endl;
    std::cout << "current login user => id:" << g_user.getId() << " name:" << g_user.getName() << std::endl;
    std::cout << "----------------------friend list---------------------" << std::endl;
    // 打印好友列表
    if (!g_friends.empty())
    {
        for (User &user : g_friends)
        {
            std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        }
    }
    std::cout << "----------------------group list----------------------" << std::endl;
    if (!g_groups.empty())
    {
        for (Group &group : g_groups)
        {
            std::cout << "group : " << std::endl;
            std::cout << group.getId() << " " << group.getGroupName() << " " << group.getGroupDesc() << std::endl;
            for (GroupUser &user : group.getGroupUser())
            {
                std::cout << "groupuser : " << std::endl;
                std::cout << user.getId() << " " << user.getName() << " " << user.getState()
                     << " " << user.getGroupRole() << std::endl;
            }
        }
    }
}

// 各个响应的业务处理函数
void respondLogin(json& response) {
    // 如果登录失败（返回的errno不为0），则输出errmsg
    int err = response["errno"];
    if (err != 0) {
        // 登录失败
        g_isLoginSuccess = false;
        std::string errmsg = response["errmsg"];
        std::cerr << "respondLogin >> errmsg = " << errmsg << std::endl;
    }
    else {
        // 登录成功
        std::cout << "respondLogin >> success to login" << std::endl;

        // 重置用户信息、好友信息、所在群信息等全局变量，避免logoff后值有错误
        g_user.clearUser();
        g_friends.clear();
        g_groups.clear();
        g_isLoginSuccess = true;
        g_isEnterUserInterface = true; // 从登录界面到用户界面

        // 记录：用户信息，好友信息，所在群的群组信息
        g_user.setId(response["userid"].get<int>());
        g_user.setName(response["username"]);
        g_user.setState(response["userstate"]);

        if (response.contains("friends")) {
            std::vector<std::string> vec = response["friends"];
            for (std::string& vec1 : vec) {
                json js = json::parse(vec1);
                User guy;
                guy.setId(js["friendid"].get<int>());
                guy.setName(js["friendname"].get<std::string>());
                guy.setState(js["friendstate"].get<std::string>());
                g_friends.push_back(guy);
            }
        }

        if (response.contains("groups")) {
            std::vector<std::string> vec = response["groups"];
            std::cout << "enter groups" << std::endl;
            for (std::string& vec1 : vec) {
                json js = json::parse(vec1);
                Group group;
                group.setId(js["groupid"].get<int>());
                group.setGroupName(js["groupname"]);
                group.setGroupDesc(js["groupdesc"]);

                std::cout << "enter groupusers" << std::endl;
                // 记录群组中的群成员
                std::vector<std::string> groupUsers = js["groupusers"];
                for (std::string& groupUser : groupUsers) {
                    json js1 = json::parse(groupUser);
                    GroupUser user;
                    user.setId(js1["gmemid"].get<int>());
                    user.setName(js1["gmemname"]);
                    user.setState(js1["gmemstate"]);
                    user.setGroupRole(js1["gmemrole"]);
                    group.getGroupUser().push_back(user);
                }

                g_groups.push_back(group);
            }
        }
        
        // 显示用户相关的信息
        showUserInfo();

        // 拉取未读的离线消息（单聊+群聊）并显示
        if (response.contains("offlinemsg")) {
            std::vector<std::string> offlinemsgs = response["offlinemsg"];
            for (std::string& offlinemsg : offlinemsgs) {
                json js = json::parse(offlinemsg);
                int msgId = js["msgid"].get<int>();
                int fromId = js["fromid"].get<int>();
                std::string time = js["time"].get<std::string>();
                std::string msg = js["msg"].get<std::string>();
                std::string fromUserName = js["fromusername"].get<std::string>();

                if (ONE_CHAT_MSG == msgId) { // 单聊
                    std::cout << "----------------------Uread onechat message----------------------" << std::endl;
                    // 打印离线消息
                    // Unread message >> time : xxx, groupid : xxx, groupdesc : xxx, userid : xxx, msg : xxx
                    std::cout << "One Chat unread message >> time : " << time << ", fromid : " << fromId << ", fromUserName : " << fromUserName << ", msg : " << msg << std::endl;
                }
                else if (GROUP_CHAT_MSG == msgId) { // 群聊
                    std::cout << "----------------------Uread groupchat message----------------------" << std::endl;
                    // 打印离线消息
                    // Unread message >> time : xxx, groupid : xxx, groupdesc : xxx, userid : xxx, msg : xxx
                    std::cout << "Group Chat unread message >> time : " << time << ", groupid : " << js["groupid"].get<int>() << ", fromid" << fromId << ", fromUserName : " << fromUserName << ", msg : " << msg << std::endl;
                }
            }
        }
        std::cout << "======================================================" << std::endl;
    }
}

// 注册响应的业务处理函数
void respondReg(json& response) {
    // 注册失败
    if (0 != response["errno"].get<int>()) {
        std::cerr << "respondReg >> register failed, errmsg : " << response["errmsg"] << std::endl;
    }
    // 注册成功
    else {
        std::cout << "respondReg >> success to register" << std::endl;
    }
}

// 添加好友的响应函数
void respondAddFriend() {
    std::cout << "respondAddFriend >> success to add friend" << std::endl;
}

// 建群的响应函数
void respondCreateGroup() {
    std::cout << "respondCreateGroup >> success to create group" << std::endl;
}

// 加群的响应函数
void respondJoinGroup() {
    std::cout << "respondJoinGroup >> success to join in group" << std::endl;
}

// 注销的响应函数
void respondLogOff() {
    // std::cout << "respondLogOff >> success to log off" << std::endl;
}

// 用户界面
void userInterface(int sockfd) {
    // 打印主页面
    help();

    // 从登录界面到用户界面
    while (g_isEnterUserInterface) {
        // 输入命令
        char buffer[1024] = {0};
        std::cin.getline(buffer, 1024);
        std::string commandBuf(buffer);

        // 到commandHandlerMap中查找相应的业务处理函数，并调用
        int idx = commandBuf.find(":");
        std::string command;
        if (-1 == idx) {
            command = commandBuf;
        }
        else {
            command = commandBuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        // 如果没有找到该业务
        if (it == commandHandlerMap.end()) {
            std::cerr << "command is invalid" << std::endl;
            continue;
        }
        // 如果找到了该业务，则执行该业务的处理函数
        it->second(sockfd, commandBuf.substr(idx + 1, commandBuf.size() - idx));
    }
}

// 线程执行函数
// 接收线程
void recvThreadFunc(int sockfd) {
    for (;;) {
        // 调用recv函数接收服务端发来的数据
        char buf[1024] = {0};
        int len =  recv(sockfd, buf, 1024, 0);
        if (-1 == len || 0 == len) {
            close(sockfd);
            exit(-1);
        }

        // 调用json::parse函数进行解析
        json response = json::parse(buf);
        int msgId = response["msgid"].get<int>();
        std::cout << "msgid : " << msgId << std::endl;
        if (LOGIN_MSG_ACK == msgId) {
            respondLogin(response);
            // 通知主线程，登录响应处理完成
            sem_post(&g_rwsem);
            continue;
        }

        if (REG_MSG_ACK == msgId) {
            respondReg(response);
            // 通知主线程，注册响应处理完成
            sem_post(&g_rwsem);
            continue;
        }

        // 建群
        if (CREATE_GROUP_MSG_ACK == msgId) {
            respondCreateGroup();
            continue;
        }

        // 加群
        if (JOIN_GROUP_MSG_ACK == msgId) {
            respondJoinGroup();
            continue;
        }

        if (ADD_FRIEND_MSG_ACK == msgId) {
            respondAddFriend();
            continue;
        }

        if (ONE_CHAT_MSG == msgId) {
            // 将msg解析出来打印输出
            // One chat message >> from : xxx, msg : xxx
            std::cout << "One chat message >> fromid : " << response["fromid"].get<int>() << ", fromname : " << response["fromname"].get<std::string>() 
                    << ", msg : " << response["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgId) {
            // 将msg解析出来打印输出
            // std::cout << "<<<<<<<<<<<<<<<<<<<<<dayinshuchu>>>>>>>>>>>>>>>>>>>>>" << std::endl;
            // Group chat message >> from : xxx, msg : xxx
            std::cout << "Group chat message >> groupid : " << response["groupid"].get<int>() << ", fromid : " << response["fromid"].get<int>() << ", fromname : " << response["fromname"].get<std::string>() 
                    << ", msg : " << response["msg"].get<std::string>() << std::endl;
            continue;
        }

        // 注销
        if (LOGOFF_MSG_ACK == msgId) {
            respondLogOff();
            continue;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "command invalid! example : ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // tcp客户端编程，连接服务器
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd) {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }

    // 填充sockaddr_in结构体
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // 调用connect函数连接server
    if (-1 == ::connect(sockfd, (sockaddr*)&server, sizeof(sockaddr_in))) {
        std::cerr << "connect server error" << std::endl;
        ::close(sockfd);
        exit(-1);
    }

    // 初始化读写线程通信用的信号量
    sem_init(&g_rwsem, 0, 0);

    // 连接服务器成功，启动接收子线程
    std::thread recvThread(recvThreadFunc, sockfd);
    recvThread.detach();

    // 向服务器发送请求
    for (;;) {
        // 显示首页面菜单 登录、注册、退出
        std::cout << "========================" << std::endl;
        std::cout << "1. login" << std::endl;
        std::cout << "2. register" << std::endl;
        std::cout << "3. quit" << std::endl;
        std::cout << "========================" << std::endl;

        // 输入
        std::cout << "choice : " << std::endl;
        int choice = 0;
        std::cin >> choice;
        std::cin.get(); // 读掉回车符
        switch (choice) {
        case LOGIN : {
            // 输入
            int id = 0;
            char password[50] = {0};
            std::cout << "userid : ";
            std::cin >> id;
            std::cin.get();
            std::cout << "password : ";
            std::cin.getline(password, 50);

            // 封装json串并发送
            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = password;
            std::string jsString = js.dump();
            int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
            if (len == -1) {
                std::cerr << "login >> send data : %s (error)" << jsString << std::endl;
            }

            // 等待接收线程接收登录响应，登录成功，将g_isLoginSuccess置为true
            g_isLoginSuccess = false;
            g_isEnterUserInterface = false;

            // 等待信号量，由子线程处理完登录的响应消息后，通知这里
            sem_wait(&g_rwsem);

            // 进入聊天主菜单页面
            if (g_isLoginSuccess) {
                std::cout << "enter user interface" << std::endl;
                g_isEnterUserInterface = true;
                userInterface(sockfd);
            }
            break;
        }
        // 注册
        case REG : {
            // 输入
            char name[50] = {0};
            char password[50] = {0};
            std::cout << "username : ";
            std::cin.getline(name, 50);
            std::cout << "password : ";
            std::cin.getline(password, 50);

            // 封装json串并发送
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = password;
            std::string jsString = js.dump();
            int len = ::send(sockfd, jsString.c_str(), strlen(jsString.c_str()) + 1, 0);
            if (len == -1) {
                std::cerr << "reg >> send data : %s (error)" << jsString << std::endl;
            }

            // 等待信号量，由子线程处理完登录的响应消息后，通知这里
            sem_wait(&g_rwsem);

            // 回到登录界面
            break;
        }
        // 退出
        case EXIT :
            close(sockfd);
            sem_destroy(&g_rwsem);
            exit(0);
            break;
        default:
            std::cerr << "invalid input! place input again" << std::endl;
            break;
        }
    }

    return 0;
}