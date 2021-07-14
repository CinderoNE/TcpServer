#pragma once
#ifndef _TCP_CONNECTION_H_
#define _TCP_CONNECTION_H_

#include"InetAddress.h"
#include"Timestamp.h"
#include"Buffer.h"
#include"http/HttpContext.h" //for http server

#include<functional>
#include<memory>

class Channel;
class Socket;
class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	using SPTcpConnection = std::shared_ptr<TcpConnection>;

	using ConnectionCallback = std::function<void(const SPTcpConnection&)>;
	using MessageCallback = std::function<void(const SPTcpConnection&,Buffer*,Timestamp)>;
	using CloseCallback = std::function<void(const SPTcpConnection&)>;
	using SendCompleteCallback = std::function<void(const SPTcpConnection&)>;

	TcpConnection(EventLoop* loop, int client_fd, const InetAddress& client_addr, const std::string& conn_name);
	~TcpConnection();

	void set_connection_callback(const ConnectionCallback& cb) {
		connection_callback_ = cb;
	}

	void set_message_callback(const MessageCallback& cb) {
		message_callback_ = cb;
	}

	void set_close_callback(const CloseCallback& cb) {
		close_callback_ = cb;
	}

	void set_send_complete_callback(const SendCompleteCallback& cb) {
		send_complete_callback_ = cb;
	}

	void set_context(const HttpContext& context) {
		context_ = context;
	}

	EventLoop* GetLoop() {
		return loop_;
	}

	bool Connected() {
		return state_ == kConnected;
	}
	void Send(const std::string& msg);
	void Shutdown();

	void SetTcpNodelay(bool on);

	void ConnectionEstablished();

	void ConnectionDestoryed();

	const std::string& name()const {
		return name_;
	}

	const InetAddress& client_addr() const {
		return client_addr_;
	}

	HttpContext* context() {
		return &context_;
	}
	
private:
	//KpeerFIN：对端主动关闭连接
	enum State{kConneting,kConnected,kDisconnecting,kDisconnected,kPeerFIN};

	void set_state(State state) {
		state_ = state;
	}
	void HandleRead(Timestamp receive_time);
	void HandleWrite();
	void HandleClose();
	void ForceClose();  //强制关闭
	void HandleError();
	void SendInLoop(const std::string& msg);
	void ShutdownInLoop();

	EventLoop* loop_;
	std::unique_ptr<Channel> channel_;
	std::unique_ptr<Socket> socket_;
	InetAddress client_addr_;
	std::string name_;
	State state_;

	ConnectionCallback connection_callback_;
	MessageCallback message_callback_;
	CloseCallback close_callback_;
	SendCompleteCallback send_complete_callback_;
	Buffer read_buffer_;
	Buffer write_buffer_;

	HttpContext context_; //for http server

	

};


#endif // !_TCP_CONNECTION_H_

