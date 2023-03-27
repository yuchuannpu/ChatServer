#include "redis.hpp"

#include <iostream>
#include <thread>

// redis-server的配置信息
// 后面要从conf配置文件中读取
const char* REDIS_IP = "127.0.0.1";
const int REDIS_PORT = 6379;

// 构造函数
Redis::Redis()
{}

// 析构函数
Redis::~Redis() {
    // 释放资源
}

// 连接redis-server
bool Redis::connectRedis() {
    // 调用redisConnect函数让publish和subscribe的上下文（redis-cli）和redis-server建立连接
    pubContext_ = redisConnect(REDIS_IP, REDIS_PORT);
    if (pubContext_ == nullptr) {
        std::cerr << "(pubContext_) connect redis-server error" << std::endl;
        return false;
    }

    subContext_ = redisConnect(REDIS_IP, REDIS_PORT);
    if (subContext_ == nullptr) {
        std::cerr << "(subContext_) connect redis-server error" << std::endl;
        return false;
    }

    // 开启一个线程，在该线程中监听通道上的消息（该线程一直阻塞在监听该通道上是否有消息到达）
    std::thread t([&]() {
        getChannelMsgFunc();
    });
    t.detach();

    return true;
}

// 订阅channel
bool Redis::subscribeChannel(int channel) {
    // 调用redisAppendCommand函数将命令组装好后写到缓存
    if (REDIS_ERR == redisAppendCommand(subContext_, "SUBSCRIBE %d", channel)) {
        std::cerr << "subscribe command invalid"  << std::endl;
        return false;
    }

    // 调用redisBufferWrite将该命令从本地缓存中发送到redis-server上
    int done = 0;
    while (done != 0) {
        if (REDIS_ERR == redisBufferWrite(subContext_, &done)) {
            std::cerr << "subscribe command invalid"  << std::endl;
            return false;
        }
    }

    return true;
}

// 取消订阅channel
bool Redis::unsubscribeChannel(int channel) {
    // 调用redisAppendCommand函数将命令组装好后写到缓存
    if (REDIS_ERR == redisAppendCommand(subContext_, "UNSUBSCRIBE %d", channel)) {
        std::cerr << "unsubscribe command invalid"  << std::endl;
        return false;
    }

    // 调用redisBufferWrite将该命令从本地缓存中发送到redis-server上
    int done = 0;
    while (done != 0) {
        if (REDIS_ERR == redisBufferWrite(subContext_, &done)) {
            std::cerr << "unsubscribe command invalid"  << std::endl;
            return false;
        }
    }

    return true;
}

// 向channel发送消息
bool Redis::pubMegToChannel(int channel, std::string msg) {
    // 调用redisCommand函数，向redis-server的通道中发送消息（PULISH channelid “xxx”）
    redisReply* reply = (redisReply*)redisCommand(pubContext_, "PUBLISH %d %s", channel, msg.c_str());
    if (nullptr == reply) {
        std::cerr << "publish command failed!" << std::endl;
        return false;
    }
    freeReplyObject(reply);

    return true;
}

// channel中有消息到达后要调用的回调函数
void Redis::getChannelMsgFunc() {
    // 调用redisGetReply函数监听subscribe context上是否有数据到达（redisGetReply是以阻塞的方式等待命令执行的结果）
    redisReply* reply = nullptr;
    while (REDIS_OK == redisGetReply(subContext_, (void **)&reply)) {
        // 如果有数据到达，则调用回调函数给业务层上报通道上到达的消息
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            getChannelMsgHandler_(atoi(reply->element[1]->str), reply->element[2]->str);
        }

        freeReplyObject(reply);
    }
    
    std::cerr << ">>>>>>>>>>>>> getChannelMsgFunc quit <<<<<<<<<<<<<" << std::endl;
}

// 设置回调函数对象
void Redis::setChannelMsgHandler(std::function<void(int, std::string)> callBack) {
    this->getChannelMsgHandler_ = callBack;
}