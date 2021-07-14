#include "HttpResponse.h"

#include"../Buffer.h"

void HttpResponse::appendToBuffer(std::string* output) const
{
    char buf[32];
    // 构造响应行
    snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
    output->append(buf);
    output->append(statusMessage_); // 响应码对应的文本信息
    output->append("\r\n");

    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
    output->append(buf);
    if (closeConnection_)
    {
        output->append("Connection: close\r\n");
    }
    else
    {
        
        output->append("Connection: Keep-Alive\r\n");
    }

    // 迭代构造响应头
    for (const auto& header : headers_)
    {
        output->append(header.first);
        output->append(": ");
        output->append(header.second);
        output->append("\r\n");
    }

    output->append("\r\n");
    output->append(body_);
}
