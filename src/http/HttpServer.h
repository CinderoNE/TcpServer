#pragma once
#ifndef _HTTP_SERVER_H_
#define _HTTP_SERVER_H_

#include"../TcpServer.h"


class HttpRequest;
class HttpResponse;

class HttpServer
{
public:
	using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;
public:
	HttpServer(EventLoop* loop, const InetAddress& listen_addr, int thread_num = 0);
	~HttpServer();

	EventLoop* GetLoop() const {  //base loop
		return tcp_server_.GetLoop();
	}

	void set_http_callback(const HttpCallback& cb) {
		http_callback_ = cb;
	}

	void Start() {
		tcp_server_.Start();
	}
private:

	void OnConnection(const TcpServer::SPTcpConnection& conn);
	void OnMessage(const TcpServer::SPTcpConnection& conn, Buffer* buf, Timestamp receive_time);
	bool OnRequest(const TcpServer::SPTcpConnection& conn, const HttpRequest& http_request);

	TcpServer tcp_server_;
	
	HttpCallback http_callback_; //用户回调

	
};


#endif // !_HTTP_SERVER_H_


