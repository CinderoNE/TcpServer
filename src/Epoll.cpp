#include"Epoll.h"

#include<sys/epoll.h>
#include<unistd.h>
#include<iostream>

constexpr int kMaxEvent = 5000;

using std::cout;
using std::endl;

Epoll::Epoll():event_list_(kMaxEvent)
{
	epollfd_ = epoll_create(kMaxEvent);
	if (epollfd_ == -1) {
		std::cerr << "epoll_create error" << endl;
		exit(1);
	}
}

Epoll::~Epoll()
{
	close(epollfd_);
}

void Epoll::epoll(ChannelList& active_channel_list,int timeout_ms)
{
	int num_events = epoll_wait(epollfd_, &*event_list_.begin(), static_cast<int>(event_list_.size()), timeout_ms);
	if (num_events == -1) {
		std::cerr << "epoll_wait error" << endl;
		exit(-1);
	}
	else if (num_events == 0) {
		cout << epollfd_ << " : nothing happend,channel size : " << channel_map_.size() << endl;
	}

	for (int i = 0; i < num_events; ++i) {
		int events = event_list_[i].events;
		Channel* pchannel = static_cast<Channel*>(event_list_[i].data.ptr);
		//int fd = pchannel->GetFd();

		pchannel->set_revents(events);
		active_channel_list.push_back(pchannel);
	}

	if (static_cast<size_t>(num_events) == event_list_.size()) {
		cout << "eventlist_ resize" << endl;
		//手动扩容1.5倍
		event_list_.resize(event_list_.size() + (event_list_.size() >> 1));
	}
}

void Epoll::AddChannel(Channel* pchannel)
{
	int fd = pchannel->GetFd();
	int events = pchannel->GetEvents();

	struct epoll_event event;
	event.events = events;
	event.data.ptr = pchannel;

	if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) == -1) {
		std::cerr << "epoll_add error" << std::endl;
	}
	channel_map_[fd] = pchannel;
}

void Epoll::RemoveChannel(Channel* pchannel)
{
	/*std::cout << "RemoveChannel  fd : "<< pchannel->GetFd() << " events : " << pchannel->GetEvents() << std::endl;*/
	if (!pchannel->deleted()) {
		int fd = pchannel->GetFd();
		int events = pchannel->GetEvents();

		struct epoll_event event;
		event.events = events;
		event.data.ptr = pchannel;

		channel_map_.erase(fd);
		
		if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) == -1) {
			perror("epoll_del error");
		}
		pchannel->set_deleted(true);
	}	
}

void Epoll::UpdateChannel(Channel* pchannel)
{
	/*std::cout << "UpdateChannel fd : " << pchannel->GetFd() << " events : " << pchannel->GetEvents() << std::endl;*/
	//无事件从epoll中移除
	if (pchannel->IsNoneEvent()) {
		RemoveChannel(pchannel);
	//曾经移除过，重新添加
	}else if (pchannel->deleted()) {
		AddChannel(pchannel);
	}
	//正常更新
	else {
		int fd = pchannel->GetFd();
		int events = pchannel->GetEvents();

		struct epoll_event event;
		event.events = events;
		event.data.ptr = pchannel;


		if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) == -1) {
			std::cerr << "epoll_del error" << std::endl;
		}
	}

	
}
