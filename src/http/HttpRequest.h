#pragma once
#ifndef _HTTP_REQUESET_H_
#define _HTTP_CONTEXT_H_

#include"../Timestamp.h"

#include<string>
#include<map>
#include <cassert>

class HttpRequest
{
public:
    // 请求方法
    enum Method
    {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    // 协议版本
    enum Version
    {
        kUnknown, kHttp10, kHttp11
    };

    HttpRequest()
        : method_(kInvalid),
        version_(kUnknown)
    {
    }

    void setVersion(Version v)
    {
        version_ = v;
    }

    Version getVersion() const
    {
        return version_;
    }

    bool setMethod(const char* start, const char* end)
    {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if (m == "GET")
        {
            method_ = kGet;
        }
        else if (m == "POST")
        {
            method_ = kPost;
        }
        else if (m == "HEAD")
        {
            method_ = kHead;
        }
        else if (m == "PUT")
        {
            method_ = kPut;
        }
        else if (m == "DELETE")
        {
            method_ = kDelete;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const
    {
        return method_;
    }

    const char* methodString() const
    {
        const char* result = "UNKNOWN";
        switch (method_)
        {
        case kGet:
            result = "GET";
            break;
        case kPost:
            result = "POST";
            break;
        case kHead:
            result = "HEAD";
            break;
        case kPut:
            result = "PUT";
            break;
        case kDelete:
            result = "DELETE";
            break;
        default:
            break;
        }
        return result;
    }

    void setPath(const char* start, const char* end)
    {
        path_.assign(start, end);
    }

    const std::string& path() const
    {
        return path_;
    }

    void setQuery(const char* start, const char* end)
    {
        query_.assign(start, end);
    }

    const std::string& query() const
    {
        return query_;
    }

    void setReceiveTime(Timestamp t)
    {
        receiveTime_ = t;
    }

    Timestamp receiveTime() const
    {
        return receiveTime_;
    }

    void addHeader(const char* start, const char* colon, const char* end)
    {
        std::string field(start, colon);
        ++colon;
        // 跳过 : 后面的空格
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }
        std::string value(colon, end);
        // 去除空格
        while (!value.empty() && isspace(value[value.size() - 1]))
        {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;  // 将解析出来的头信息放入map中
    }

    std::string getHeader(const std::string& field) const
    {
        std::string result;
        std::map<std::string, std::string>::const_iterator it = headers_.find(field);
        if (it != headers_.end())
        {
            result = it->second;
        }
        return result;
    }

    const std::map<std::string, std::string>& headers() const
    {
        return headers_;
    }

    void swap(HttpRequest& that)
    {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
    }

private:
    Method method_; // 请求方法
    Version version_; // 协议版本 1.0/1.1
    std::string path_; // 请求路径
    std::string query_;  // 请求参数
    Timestamp receiveTime_; // 请求时间  eg:  /search  ?  hl=zh-CN&source=hp&q=domety&aq=f&oq=
    std::map<std::string, std::string> headers_;  // header列表

};



#endif //_HTTP_REQUESET_H_

