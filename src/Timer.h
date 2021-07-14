#pragma once
#ifndef _TIMER_H_
#define _TIMER_H_

#include"Timestamp.h"

#include<functional>
#include<atomic>




class Timer
{
public:
	using TimerCallback = std::function<void()>;
	using TimerId = uint64_t;
public:
	Timer(Timestamp when,const TimerCallback &cb);
	~Timer() {  }
	
	TimerId timer_id() const {
		return timer_id_;
	}

	Timestamp expired_time() const { return expired_time_; }

	void set_expired_time(Timestamp when) {
		expired_time_ = when;
	}

	bool IsValid() const { return Timestamp::now() < expired_time_; }

	TimerCallback get_timer_callback ()const {
		return timer_callback_;
	}
	// kDeleted 取消计时器（不执行到期回调），不会重新添加
	// kReset //取消计时器,到期后执行reset回调(需要设置者自己重新添加定时器)
	enum State { kNormal, kDeleted, kReset };

	void SetNormal() {
		state_ = kNormal;
	}

	void SetDeleted() {
		state_ = kDeleted;
	}

	bool IsDeleted() const {
		return state_ == kDeleted;
	}

	bool IsReset() const {
		return state_ == kReset;
	}
	
	//run_cb : reset之前是否运行回调函数
	void SetResetAt(Timestamp when,bool run_cb) {
		state_ = kReset;
		next_expired_time_ = when;
		reset_run_cb_ = run_cb;
	}

	Timestamp next_expired_time() const {
		return next_expired_time_;
	}

	void run() {
		timer_callback_();
	}

	bool reset_run_cb()const {
		return reset_run_cb_;
	}
	
private:
	TimerId timer_id_;
	Timestamp expired_time_;
	TimerCallback timer_callback_;

	State state_;
	
	Timestamp next_expired_time_;  //重新添加计时器的到期时间
	bool reset_run_cb_;

private:
	static std::atomic_uint_fast64_t sequence; //生成timer_id
};


#endif // !_TIMER_H_



