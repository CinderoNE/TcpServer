#pragma once
#ifndef _HTTP_CONTEXT_H_
#define _HTTP_CONTEXT_H_

#include"../Buffer.h"
#include"../Timestamp.h"
#include"../Timer.h"
#include"HttpRequest.h"

#include<memory>
#include<iostream>

class HttpContext
{
public:
    using ShutdownTimer = std::weak_ptr<Timer>;
    using SPTimer = std::shared_ptr<Timer>;

    enum HttpRequestParseState
    {
        kParseRequestLine,
        kParseHeaders,
        kParseBody,
        kParseFinish,
    };

    HttpContext()
        :state_(kParseRequestLine)
    {
    }

	bool ParseRequest(Buffer* buf, Timestamp receiveTime);

    bool ParseFinish() const {
        return state_ == kParseFinish;
    }

    const HttpRequest& request() const
    {
        return request_;
    }

    HttpRequest& request()
    {
        return request_;
    }

    void reset()
    {
        state_ = kParseRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    void set_shutdown_timer(const SPTimer &timer) {
        shutdown_timer = timer;
    }

    void ResetShutdownTimer(double delay) {
        if (!shutdown_timer.expired()) {
            shutdown_timer.lock()->SetResetAt(NowAfterSeconds(delay), false); //重置关闭连接时间   
        }
    }

    bool ShutdownTimerExpired() const {
        return shutdown_timer.expired();
    }
    
private:
    //解析请求行
    bool ParseRequestLine(const char* begin, const char* end);
private:
    HttpRequestParseState state_;  //当前解析状态
    HttpRequest request_; //解析结果

    ShutdownTimer shutdown_timer; //weak_ptr,避免循环引用
};


#endif // !_HTTP_CONTEXT_H_


