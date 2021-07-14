#include "Timer.h"

#include<iostream>
std::atomic_uint_fast64_t Timer::sequence{0};

Timer::Timer(Timestamp when,const TimerCallback& cb)
	:timer_id_(++sequence),
	expired_time_(when),
	timer_callback_(cb),
	state_(kNormal)
{
}




