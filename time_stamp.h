#ifndef __TIME_STAMP__
#define __TIME_STAMP__
#include <string>
#include <utility>
#include <sys/time.h>
#include <boost/operators.hpp>
#include <boost/noncopyable.hpp>
namespace krys
<%


class time_stamp: public boost::less_than_comparable<time_stamp>
{
public:

	/*-----------------------------------------constructors--------------------------------------------*/
	time_stamp () :micro_seconds_since_1970_ (0) { }


	explicit time_stamp (uint64_t micro_seconds_since_1970) 
		:micro_seconds_since_1970_ (micro_seconds_since_1970) { }
	/*----------------------------------------------END------------------------------------------------*/


	void swap (time_stamp that)
	{
		std::swap (micro_seconds_since_1970_, that.micro_seconds_since_1970_);
	}

	inline double time_difference (time_stamp high, time_stamp low)
	{
		int64_t diff = high.micro_seconds_since_1970_ - low.micro_seconds_since_1970_;
		return static_cast<double> (diff) / 1000000;
	}

	inline time_stamp add_time (time_stamp time, double secoonds)
	{
		int64_t delta = static_cast<int64_t> (secoonds * 1000000);

		return time_stamp (time.micro_seconds_since_1970_ + delta);
	}

	std::string to_string () const
	{
		std::string buf;
		buf.resize (24);

		time_t seconds = this->micro_seconds_since_1970_ / 1000000;
		int micro_seconds = this->micro_seconds_since_1970_% 1000000;
		struct tm time_value;
		gmtime_r (&seconds, &time_value);

		snprintf (const_cast<char*> (buf.c_str()), 24 + 1, "%4d%02d%02d %02d:%02d:%02d.%06d", 
				time_value.tm_year + 1900, time_value.tm_mon + 1, time_value.tm_mday, 
				time_value.tm_hour, time_value.tm_min, time_value.tm_sec, 
				micro_seconds);

		return buf;
	}

	bool valid () const;

	friend bool operator< (krys::time_stamp lhs, krys::time_stamp rhs)
	{
		return lhs.micro_seconds_since_1970_ < rhs.micro_seconds_since_1970_;
	}

	friend bool operator== (krys::time_stamp lhs, krys::time_stamp rhs)
	{
		return lhs.micro_seconds_since_1970_ == rhs.micro_seconds_since_1970_;
	}

public:
	static time_stamp invalid ()
	{
		return time_stamp ();
	}

	static time_stamp now ()
	{
		struct timeval tv;
		gettimeofday (&tv, nullptr);
		return time_stamp (tv.tv_sec * 1000000 + tv.tv_usec);
	}
private:
	uint64_t micro_seconds_since_1970_;

private:
};

%>

#endif /*__TIME_STAMP__*/
