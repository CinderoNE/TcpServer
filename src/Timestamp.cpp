#include "Timestamp.h"

#include<chrono>
#include<sstream>
#include <iomanip>

constexpr int Timestamp::kNanoSecondsPerSecond; //1000 * 1000 * 1000
constexpr int Timestamp::kNanoSecondsPerMilliSecond; //1000 * 100



Timestamp::Timestamp() :
	nano_seconds_since_epoch_(0) {

}

Timestamp::Timestamp(int64_t nanoseconds) :
	nano_seconds_since_epoch_(nanoseconds) {

}

int64_t Timestamp::NowMilli() {
	return NowNano() / kNanoSecondsPerMilliSecond;
}

int64_t Timestamp::NowNano() {
	return std::chrono::system_clock::now().time_since_epoch().count();
}

Timestamp Timestamp::now()
{
	return Timestamp(NowNano());
}

std::string Timestamp::ToString() const
{
	int64_t seconds = nano_seconds_since_epoch_ / kNanoSecondsPerSecond;
	int64_t nano_seconds = nano_seconds_since_epoch_ % kNanoSecondsPerSecond;
	int64_t milli_seconds = nano_seconds / kNanoSecondsPerMilliSecond;
	std::stringstream ss;
	ss << seconds << "." << milli_seconds;
	return ss.str();
}

std::string Timestamp::ToFormattedString() const
{
	using duration = typename std::chrono::system_clock::duration;
	using time_point = typename std::chrono::system_clock::time_point;

	time_t t = std::chrono::system_clock::to_time_t(time_point(duration(nano_seconds_since_epoch_)));
	std::stringstream ss;
	ss << std::put_time(std::localtime(&t), "%Y-%m-%d %X");

	return ss.str();
}