#pragma once
#ifndef _INET_ADDRESS_H_
#define _INET_ADDRESS_H_

#include<netinet/in.h>
#include<string>

class InetAddress
{
public:
	InetAddress();

	explicit InetAddress(uint16_t port);

	InetAddress(const std::string& ip , uint16_t port);

	InetAddress(const sockaddr_in& addr):
		addr_(addr){}

	const struct sockaddr_in& get_sockaddr_inet() const {
		return addr_;
	}

	void set_sockaddr_inet(const struct sockaddr_in& addr) {
		addr_ = addr;
	}

	std::string ToString()const;

private:
	struct sockaddr_in addr_;
};

#endif // !_INET_ADDRESS_H_



