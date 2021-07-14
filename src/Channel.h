#pragma once
#ifndef _CHANNEL_H_
#define _CHANNEL_H_


#include"Timestamp.h"


#include<functional>


/*
每个Channel对象只负责一个fd的IO事件分发，
将不同的IO事件分发为不同的回调
*/

class Channel
{
public:
	using EventCallback = std::function<void()>;
	using ReadEventCallback = std::function<void(Timestamp)>;

	Channel(int fd);
	~Channel();

	int GetFd() { 
		return fd_; 
	}

	uint32_t GetEvents() {
		return events_;
	}

	void set_revents(uint32_t events) {
		revents_ = events;
	}

	void EnableRead() {
		events_ |= kReadEvent;
	}

	void EnableWrite() {
		events_ |= kWriteEvent;
	}

	void DisalbeWrite() {
		events_ &= ~kWriteEvent;
	}

	bool IsWriting() const{
		return events_ & kWriteEvent;
	}

	bool IsNoneEvent() const{
		return events_ == kNoneEvent;
	}
	void DisableAll() {
		events_ = kNoneEvent;
	}

	bool deleted ()const {
		return deleted_;
	}

	void set_deleted(bool b) {
		deleted_ = b;
	}
	void HandleEvent(Timestamp receive_time);

	void set_read_callback(const ReadEventCallback& cb) {
		read_callback_ = cb;
	}

	void set_write_callback(const EventCallback& cb) {
		write_callback_ = cb;
	}

	void set_error_callback(const EventCallback& cb) {
		error_callback_ = cb;
	}

	void set_close_callback(const EventCallback& cb) {
		close_callback_ = cb;
	}

private:

	int fd_;
	//注册事件
	uint32_t events_;
	//触发事件
	uint32_t revents_;


	//是否已经从epoll中删除
	bool deleted_;

	ReadEventCallback read_callback_;
	EventCallback write_callback_;
	EventCallback error_callback_;
	EventCallback close_callback_;

private:
	static const uint32_t kNoneEvent;
	static const uint32_t kReadEvent;
	static const uint32_t kWriteEvent;

};




#endif // !_CHANNEL_H_
