#include"Acceptor.h"

#include"EventLoop.h"
#include"InetAddress.h"
#include<iostream>
#include<unistd.h>
Acceptor::Acceptor(EventLoop* loop, const InetAddress& accept_addr):
	loop_(loop),
	accept_socket_(),
	accept_channel_(accept_socket_.sock_fd())
{
	accept_socket_.SetReuseAddr(true);
	accept_socket_.BindAddresss(accept_addr);
	accept_channel_.set_read_callback(std::bind(&Acceptor::AcceptHandler,this));
	
}

void Acceptor::AcceptHandler()
{
	loop_->AssertInLoopThread();
	InetAddress client_addr;
	int client_fd;
	//循环接收所有连接
	//新建立的socket默认是nonblocking
	while ((client_fd = accept_socket_.Accept(&client_addr)) > 0) {
		if (new_connection_callback_) {
			new_connection_callback_(client_fd, client_addr);
		}
		else {
			std::cout << "AcceptHandler close client" << std::endl;
			close(client_fd);
		}
	}
}

void Acceptor::Listen()
{
	loop_->AssertInLoopThread();
	accept_socket_.Listen();
	accept_channel_.EnableRead();
	loop_->EpollAddChannel(&accept_channel_);
}
