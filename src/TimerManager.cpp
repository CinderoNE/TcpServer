#include"TimerManager.h"

#include<memory>
#include<sys/timerfd.h>
#include<strings.h>
#include<unistd.h>
#include<iostream>
#include"EventLoop.h"


int createTimerfd()
{
	int timerfd = timerfd_create(CLOCK_MONOTONIC,
		TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
	{
		std::cerr << "Failed in timerfd_create" << std::endl;
	}
	return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
	int64_t nanoseconds = when.nano_second_since_epoch() - Timestamp::NowNano();
	//最少1毫秒
	if (nanoseconds < Timestamp::kNanoSecondsPerMilliSecond)
	{
		nanoseconds = Timestamp::kNanoSecondsPerMilliSecond;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
		nanoseconds / Timestamp::kNanoSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(
		nanoseconds % Timestamp::kNanoSecondsPerSecond);
	return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
	// wake up loop by timerfd_settime()
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret)
	{
		std::cerr << "timerfd_settime()"<<std::endl;
	}
}

//将定时器功能和普通IO同样看待
TimerManager::TimerManager(EventLoop* loop):
	loop_(loop),
	timerfd_(createTimerfd()),
	timerfd_channel_(timerfd_)
{
	timerfd_channel_.EnableRead();
	timerfd_channel_.set_read_callback(std::bind(&TimerManager::HandleExpiredEvent,this,std::placeholders::_1));
	loop_->EpollAddChannel(&timerfd_channel_);
}
TimerManager::~TimerManager()
{
	close(timerfd_);
}


TimerManager::SPTimer TimerManager::AddTimer(Timestamp when, const TimerCallback & cb)
{
	SPTimer new_timer = std::make_shared<Timer>(when, std::move(cb));
	//如果是其他线程调用，转移到IO线程，可以避免锁操作（timer_queue_）
	loop_->RunInLoop(std::bind(&TimerManager::AddTimerInLoop, this, new_timer));
	
	return new_timer;
	
}


void TimerManager::AddTimerInLoop(SPTimer new_timer)
{
	Timestamp timer_timestamp = new_timer->expired_time();
	
	if (empty()) {
		timer_queue_.push(new_timer);
		resetTimerfd(timerfd_, timer_timestamp);
	}
	else {
		Timestamp earliest_timestamp = timer_queue_.top()->expired_time();
		timer_queue_.push(new_timer);
		//最早到期时间是否需要重新计算
		if (timer_timestamp < earliest_timestamp) {
			resetTimerfd(timerfd_, timer_timestamp);
		}
	}
}


void TimerManager::ResetTimer(const SPTimer& reset_timer)
{
	if (reset_timer->reset_run_cb()) {
		reset_timer->run();
	}
	reset_timer->SetNormal();
	reset_timer->set_expired_time(reset_timer->next_expired_time());
	timer_queue_.push(reset_timer);
}

void TimerManager::TimerfdRead()
{
	uint64_t how_many;
	ssize_t n = read(timerfd_, &how_many, sizeof how_many); //返回到期次数
	(void)n;
	
}

void TimerManager::HandleExpiredEvent(Timestamp receive_time)
{
	(void)receive_time;
	/*std::cout << "HandleExpiredEvent Before: " << timer_queue_.size() << std::endl;*/
	TimerfdRead();
	while (!timer_queue_.empty()) {
		SPTimer top_timer = timer_queue_.top();
		if (top_timer->IsDeleted()) {
			timer_queue_.pop();
		}
		else if (!top_timer->IsValid()) {
			if (top_timer->IsReset()) {
				timer_queue_.pop();    
				ResetTimer(top_timer);
			}
			else { //kNormal
				top_timer->run();
				timer_queue_.pop();
			}		
		}
		else { // !IsDeleted || IsValid()
			//重新设置最快到期时间
			ResetExpiredTime();
			break;
		}
	}
}

void TimerManager::ResetExpiredTime()
{
	if (!timer_queue_.empty()) {
		Timestamp timestamp = timer_queue_.top()->expired_time();
		resetTimerfd(timerfd_, timestamp);
	}
	
}
