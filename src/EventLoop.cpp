#include"EventLoop.h"

#include<iostream>
#include <sys/eventfd.h> //for wakeup
#include<unistd.h>
#include<cassert>
using std::cout;
using std::endl;


constexpr int kTimeoutMs = 10000;

//借助 eventfd 实现跨线程唤醒
int CreateEventFd()
{
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		std::cout << "Failed in eventfd" << std::endl;
		exit(1);
	}
	return evtfd;
}

EventLoop::EventLoop()
	:looping_(false),
	callingPendingFunctors_(false),
	tid_(std::this_thread::get_id()),
	epoll_(),
	active_channel_list_(),
	timer_manager_(this),
	mutex_(),
	wakeup_fd_(CreateEventFd()),
	wakeup_channel_(wakeup_fd_)
{
	wakeup_channel_.EnableRead();
	wakeup_channel_.set_read_callback(std::bind(&EventLoop::HandleRead,this));
	EpollAddChannel(&wakeup_channel_);
}

EventLoop::~EventLoop()
{
	assert(!looping_);
	close(wakeup_fd_);
}

void EventLoop::Loop()
{
	looping_ = true;
	while (looping_) {
		active_channel_list_.clear();
		epoll_.epoll(active_channel_list_, kTimeoutMs);
		for (auto pchannel : active_channel_list_) {
			pchannel->HandleEvent(Timestamp::now());
		}
		DoPendingFunctor();
	}

	cout << "stop looping" << endl;
}


TimerManager::SPTimer EventLoop::RunAt(const Timestamp& timestamp, const TimerCallback& cb) {
	return timer_manager_.AddTimer(timestamp, cb);
}

TimerManager::SPTimer EventLoop::RunAfter(double delay, const TimerCallback& cb) {
	Timestamp ts(AddSeconds(Timestamp::now(), delay));
	return RunAt(ts, cb);
}

void EventLoop::RunInLoop(Functor&& cb) {
	if (IsInLoopThread()) {
		cb();
	}
	else {
		QueueInLoop(std::move(cb));
	}
}

void EventLoop::QueueInLoop(Functor&& cb) {
	{
		std::lock_guard<std::mutex> guard(mutex_);
		pending_functors_.emplace_back(std::move(cb));
	}
	//如果正在callingPendingFunctors_,调用完后会进入epoll_wait等待，所以需要唤醒
	if (!IsInLoopThread() || callingPendingFunctors_) {
		WakeUp();
	}

}

void EventLoop::WakeUp()
{
	//cout << "WakeUp : " << wakeup_fd_ << endl;
	u_int64_t one = 1;
	ssize_t n = write(wakeup_fd_, &one, sizeof one);
	(void)n;
}

//wakeup handle
void EventLoop::HandleRead()
{
	//cout << "wakeup handle" << endl;
	u_int64_t one = 1;
	ssize_t n = read(wakeup_fd_, &one, sizeof one);
	(void)n;
}

void EventLoop::DoPendingFunctor()
{
	/*cout << "DoPendingFunctor" << endl;*/
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;

	//交换待做任务，缩小临界区
	{
		std::lock_guard<std::mutex> guard(mutex_);
		functors.swap(pending_functors_);
	}
	for (auto& functor : functors) {
		functor();
	}
	callingPendingFunctors_ = false;

}
