#include"Channel.h"

#include<sys/epoll.h>
#include<iostream>

const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const uint32_t Channel::kWriteEvent = EPOLLOUT;

//默认EPOLLET
Channel::Channel(int fd) 
	: fd_(fd),
	events_(EPOLLET),
	deleted_(false)
{
}

Channel::~Channel()
{
}

void Channel::HandleEvent(Timestamp receive_time)
{
	if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
		//std::cout << "close_callback_ : " << fd_ <<  std::endl;
		close_callback_();
	}
	if (revents_ & EPOLLERR) {
		//std::cout << "error_callback_ : " << fd_ << std::endl;
		error_callback_();
	}
	//EPOLLRDHUP 对端发送FIN,read返回0
	if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
		//std::cout << "read_callback_ : " << fd_ << std::endl;
		read_callback_(receive_time);
	}
	if (revents_ & EPOLLOUT) {
		//std::cout << "write_callback_ : " << fd_ << std::endl;
		write_callback_();
	}
}
