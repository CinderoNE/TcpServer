#pragma once

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_


#include<string>

class Timestamp {
public:

	Timestamp();
	Timestamp(int64_t nanoseconds);


	int64_t nano_second_since_epoch() const { return nano_seconds_since_epoch_; }

	std::string ToString() const;
	std::string ToFormattedString() const;
	void swap(Timestamp& that)
	{
		std::swap(nano_seconds_since_epoch_, that.nano_seconds_since_epoch_);
	}

	static int64_t NowMilli();
	static int64_t NowNano();
	static Timestamp now();
	static constexpr int kNanoSecondsPerSecond = 1000 * 1000 * 1000;
	static constexpr int kNanoSecondsPerMilliSecond = 1000 * 1000;
private:

	int64_t nano_seconds_since_epoch_;
};


inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.nano_second_since_epoch() < rhs.nano_second_since_epoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.nano_second_since_epoch() == rhs.nano_second_since_epoch();
}

inline Timestamp AddSeconds(Timestamp timestamp, double seconds) {
	int64_t delta = static_cast<int64_t>(Timestamp::kNanoSecondsPerSecond * seconds);
	return Timestamp(timestamp.nano_second_since_epoch() + delta);
}

inline Timestamp NowAfterSeconds(double seconds) {
	return AddSeconds(Timestamp::now(), seconds);
}

#endif // !_TIMESTAMP_H_





