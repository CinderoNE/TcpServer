#include"Socket.h"


#include"InetAddress.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<iostream>
#include <fcntl.h>
#include<cstring>
#include<netinet/tcp.h>


//default create nonblocking socket
Socket::Socket()
{
	sock_fd_ = socket(AF_INET,
		SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sock_fd_ < 0) {
		std::cerr << "socket error!" << std::endl;
		exit(-1);
	}
}

Socket::Socket(int sock_fd) 
	:sock_fd_(sock_fd)
{
}



Socket::~Socket()
{
	//std::cout << "Socket dtor,close socket : "  << sock_fd_ << std::endl;
	close(sock_fd_);
}

void Socket::BindAddresss(const InetAddress& addr)
{
	int ret = bind(sock_fd_, reinterpret_cast<const struct sockaddr*>(&(addr.get_sockaddr_inet())), sizeof addr);
	if (ret < 0) {
		close(sock_fd_);
		std::cerr << "bind error"<< std::endl;
	}
}

void Socket::SetReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR,
		&optval, sizeof optval);
	
}

void Socket::SetNonBlocking()
{
	int opts = fcntl(sock_fd_, F_GETFL);
	if (opts < 0) {
		std::cerr << "fcntl get error" << std::endl;
	}
	int ret = fcntl(sock_fd_, F_SETFL, opts | O_NONBLOCK);
	if (ret < 0) {
		std::cerr << "fcntl set error" << std::endl;
	}
}

void Socket::SetNoDelay(bool on)
{

	int optval = on ? 1 : 0;
	setsockopt(sock_fd_, IPPROTO_TCP, TCP_NODELAY,
		&optval, sizeof optval);

}

void Socket::ShutdownWrite()
{
	if (shutdown(sock_fd_, SHUT_WR) < 0) {
		//std::cerr << "ShutdownWrite error " << std::endl;
	}
}

void Socket::Listen()
{
	int ret = listen(sock_fd_, SOMAXCONN);
	if (ret < 0) {
		std::cerr << "listen error" << std::endl;
		close(sock_fd_);
		exit(-1);
	}
}

int Socket::Accept(InetAddress* client_addr)
{
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof addr;
	//新socket 设置 SOCK_NONBLOCK | SOCK_CLOEXEC
	int client_fd = accept4(sock_fd_, reinterpret_cast<sockaddr*>(&addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (client_fd >= 0) {
		client_addr->set_sockaddr_inet(addr);
	}else{
		int savedErrno = errno;
		switch (savedErrno)
		{
		case EAGAIN:
		case ECONNABORTED:
		case EINTR:
		case EPROTO: // ???
		case EPERM:
		case EMFILE: // per-process lmit of open file desctiptor ???
		  // expected errors
			errno = savedErrno;
			break;
		case EBADF:
		case EFAULT:
		case EINVAL:
		case ENFILE:
		case ENOBUFS:
		case ENOMEM:
		case ENOTSOCK:
		case EOPNOTSUPP:
			// unexpected errors
			perror("unexpected error of ::accept");
			break;
		default:
			perror("unknown error of ::accept ");
			break;
		}
	}
	return client_fd;
}

void Socket::Close(int sock_fd)
{
	std::cout << "close sock_fd : " << sock_fd << std::endl;
	close(sock_fd);
}

struct sockaddr_in Socket::GetLocalAddr(int sock_fd)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr, sizeof local_addr);
	socklen_t addr_len = sizeof local_addr;
	if (getsockname(sock_fd, reinterpret_cast<struct sockaddr*>(&local_addr), &addr_len) < 0) {
		std::cerr << "getsockname error" << std::endl;
	}
	return local_addr;
}

int Socket::GetSockError(int sock_fd)
{
	int optval;
	socklen_t optlen = sizeof optval;

	if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
		return errno;
	}
	return optval;

}


