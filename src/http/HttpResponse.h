#pragma once
#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include<string>
#include<map>

class Buffer;
class HttpResponse
{
public:
    enum HttpStatusCode
    {
        kUnknown,
        k200Ok = 200, // 请求正常
        k301MovedPermanently = 301, // 永久重定向
        k400BadRequest = 400, // 服务器无法理解客户端请求
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown),
        closeConnection_(close)
    {
    }

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_ = code;
    }

    void setStatusMessage(const std::string& message)
    {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on)
    {
        closeConnection_ = on;
    }

    bool closeConnection() const
    {
        return closeConnection_;
    }

    void setContentType(const std::string& contentType)
    {
        addHeader("Content-Type", contentType);
    } // 在响应中，Content-Type标头告诉客户端实际返回的内容的内容类型。

// FIXME: replace string with StringPiece
    void addHeader(const std::string& key, const std::string& value)
    {
        headers_[key] = value;
    }

    void setBody(const std::string& body)
    {
        body_ = body;
    }

    void appendToBuffer(std::string* output) const; // 将HttpResponse添加到Buffer

private:
    std::map<std::string, std::string> headers_;  // 响应头
    HttpStatusCode statusCode_; // 状态响应码
    // FIXME: add http version
    std::string statusMessage_;  // 状态响应码对应的文本信息
    bool closeConnection_;  // 是否关闭连接
    std::string body_; // 响应报文
};
#endif // !_HTTP_RESPONSE_H_



