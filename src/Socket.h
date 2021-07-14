#pragma once
#ifndef _SOCKET_H_
#define _SOCKET_H_
#endif // !_SOCKET_H_


class InetAddress;

class Socket
{
public:
	Socket();
	Socket(int sock_fd);
	~Socket();

	int sock_fd() const { return sock_fd_; }

	void BindAddresss(const InetAddress& addr);
	void SetReuseAddr(bool on);
	void SetNonBlocking();
	void SetNoDelay(bool on);
	void ShutdownWrite();

	void Listen();

	int Accept(InetAddress* client_addr);

	static void Close(int sock_fd);
	static struct sockaddr_in GetLocalAddr(int sock_fd);
	static int GetSockError(int sock_fd);
private:
	int sock_fd_;

};

