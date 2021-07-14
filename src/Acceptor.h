#pragma once
#ifndef _ACCEPTOR_H_
#define _ACCEPTOR_H_

#include<functional>
#include"Socket.h"
#include"Channel.h"

class EventLoop;
class InetAddress;


class Acceptor {
public:
	using NewConnectionCallback = std::function<void(int, const InetAddress&)>;
public:
	Acceptor(EventLoop* loop, const InetAddress& accept_addr);

	void AcceptHandler();

	void set_new_connection_callback(const NewConnectionCallback& cb) {
		new_connection_callback_ = cb;
	}

	void Listen();

private:
	EventLoop* loop_;
	Socket accept_socket_;
	Channel accept_channel_;

	NewConnectionCallback new_connection_callback_;

	
};
#endif // !_ACCEPTOR_H_
