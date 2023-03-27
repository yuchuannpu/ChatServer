#ifndef REDIS_H
#define REDIS_H

/*
Redis类，封装服务器和redis之间的操作
*/

#include <hiredis/hiredis.h>
#include <functional>
#include <string>

class Redis {
public:
    // 构造函数
    Redis();

    // 析构函数
    ~Redis();

    // 连接redis-server
    bool connectRedis();

    // 订阅channel
    bool subscribeChannel(int channel);

    // 取消订阅channel
    bool unsubscribeChannel(int channel);

    // 向channel发送消息
    bool pubMegToChannel(int channel, std::string msg);

    // channel中有消息到达后要调用的回调函数
    void getChannelMsgFunc();

    // 设置回调函数对象
    void setChannelMsgHandler(std::function<void(int, std::string)> callBack);

private:
    // 两个上下文
    // 向channel中发布消息的redis上下文（publish）
    redisContext* pubContext_;
    // 订阅channel的上下文（subscribe）
    redisContext* subContext_;

    // 回调函数对象
    // 将到达channel的数据上报给业务层
    std::function<void(int, std::string)> getChannelMsgHandler_;
};

#endif