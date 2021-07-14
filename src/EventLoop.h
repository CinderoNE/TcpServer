#pragma once
#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include"Epoll.h"
#include"Channel.h"
#include"TimerManager.h"

#include<thread>
#include<mutex>
#include<iostream>


class EventLoop {
public:
	using Functor = std::function<void()>;
	using TimerCallback = TimerManager::TimerCallback;
public:
	EventLoop();
	~EventLoop();
	void Loop();

	void AssertInLoopThread() const {
		if (!IsInLoopThread()) {
			std::cerr << "not int loop thread!" << std::endl;
		}
	}

	bool IsInLoopThread() const {
		return std::this_thread::get_id() == tid_;
	}

	void EpollAddChannel(Channel* pchannel) {
		epoll_.AddChannel(pchannel);
	}

	void EpollRemoveChannel(Channel* pchannel) {
		epoll_.RemoveChannel(pchannel);
	}

	void EpollUpdateChannel(Channel* pchannel) {
		epoll_.UpdateChannel(pchannel);
	}

	TimerManager::SPTimer RunAt(const Timestamp& timestamp, const TimerCallback& cb);

	TimerManager::SPTimer RunAfter(double delay, const TimerCallback& cb);
	

	void RunInLoop(const Functor& cb);

	void QueueInLoop(const Functor& cb);

	void WakeUp();

	void HandleRead();

	void DoPendingFunctor();

	void Quit() {
		looping_ = false;
		if (!IsInLoopThread()) {
			WakeUp();
		}
	}
	int epoll_fd() {
		return epoll_.epoll_fd();
	}

private:
	using ChannelList = std::vector<Channel*>;

	bool looping_;
	bool callingPendingFunctors_;
	std::thread::id  tid_;
	Epoll epoll_;
	ChannelList active_channel_list_;
	TimerManager timer_manager_;


	//别的线程想在IO线程中调用函数
	std::mutex mutex_; //for pending_functors_
	std::vector<Functor> pending_functors_;
	//跨线程唤醒
	int wakeup_fd_;
	Channel wakeup_channel_;

};
#endif // !_EVENT_LOOP_H_
