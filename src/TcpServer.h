#pragma once
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_


#include"InetAddress.h"
#include"TcpConnection.h"
#include"Timestamp.h"

#include<unordered_map>


class Acceptor;
class EventLoop;
class Buffer;
class EventLoopThreadPool;

class TcpServer
{
public: 
	using SPTcpConnection = std::shared_ptr<TcpConnection>;

	//用户设置，新连接建立时的回调函数
	using ConnectionCallback = std::function<void(const SPTcpConnection&)>;
	using MessageCallback = std::function<void(const SPTcpConnection&, Buffer*,Timestamp)>;
	using SendCompleteCallback = std::function<void(const SPTcpConnection&)>;
	using CloseCallback = std::function<void(const SPTcpConnection&)>;

	//thread_num
	//0 - 所有的IO事件都在baseloop中
	//1 - baseloop负责接受连接，然后交给另一个线程处理其它事件
	//N - baseloop负责接受连接，然后通过轮询交给线程池中的一个线程处理
	TcpServer(EventLoop* loop, const InetAddress& listen_addr,int thread_num = 0);
	~TcpServer();

	void Start();

	EventLoop* GetLoop() const { 
		return loop_; 
	}

	void set_connection_callback(const ConnectionCallback& cb) {
		connection_callback_ = cb;
	}

	void set_message_callback(const MessageCallback& cb) {
		message_callback_ = cb;
	}

	void set_send_complete_callback(const SendCompleteCallback& cb) {
		send_complete_callback_ = cb;
	}

	void set_close_callback(const CloseCallback& cb) {
		close_callback_ = cb;
	}

private:

	void NewConnection(int clinet_fd, const InetAddress& clinet_addr);
	void RemoveConnection(const SPTcpConnection& conn);
	void RemoveConnectionInLoop(const SPTcpConnection& conn); 

	EventLoop* loop_;  //base_loop
	std::string name_; //server_name
	std::unique_ptr<Acceptor> acceptor_;
	std::unique_ptr<EventLoopThreadPool> thread_pool_;
	bool started_;
	int conn_count_;  //连接数
	std::unordered_map<std::string, SPTcpConnection> conn_map_;//连接map

	
	ConnectionCallback connection_callback_;
	MessageCallback message_callback_;
	SendCompleteCallback send_complete_callback_;
	CloseCallback close_callback_;

};

#endif // !_TCP_SERVER_H_



