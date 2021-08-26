#pragma once
#ifndef _EPOLL_H_
#define _EPOLL_H_

#include"Channel.h"

#include<vector>
#include<sys/epoll.h>
#include<unordered_map>



class Epoll {
public:
	using ChannelList = std::vector<Channel*>;
public:
	Epoll();
	~Epoll();


	void epoll(ChannelList& active_channel_list, int timeout_ms);

	void AddChannel(Channel* pchannel);

	void RemoveChannel(Channel* pchannel);

	void UpdateChannel(Channel* pchannel);

	int epoll_fd () const {
		return epollfd_;
	}

private:

	using ChannelMap = std::unordered_map<int, Channel*>;

	std::vector<struct epoll_event> event_list_;
	
	ChannelMap channel_map_;

	int epollfd_;
};

#endif // !_EPOLL_H_
