#ifndef __TIME_STAMP__
#define __TIME_STAMP__
#include <string>
#include <boost/operators.hpp>
#include <boost/noncopyable.hpp>
namespace krys
{
	class time_stamp;
}

class krys::time_stamp: public boost::less_than_comparable<time_stamp>
{
public:
	time_stamp () :micro_seconds_since_1970_ (0) { }


	explicit time_stamp (uint64_t micro_seconds_since_1970) 
		:micro_seconds_since_1970_ (micro_seconds_since_1970) { }


	void swap (time_stamp that)
	{
		std::swap (micro_seconds_since_1970_, that.micro_seconds_since_1970_);
	}


	std::string to_string () const;
	std::string to_formatted_string () const;
	bool valid () const;

	friend bool operator< (krys::time_stamp lhs, krys::time_stamp rhs)
	{
		return lhs.micro_seconds_since_1970_ < rhs.micro_seconds_since_1970_;
	}

	friend bool operator== (krys::time_stamp lhs, krys::time_stamp rhs)
	{
		return lhs.micro_seconds_since_1970_ == rhs.micro_seconds_since_1970_;
	}

private:
	uint64_t micro_seconds_since_1970_;
};


#endif /*__TIME_STAMP__*/
