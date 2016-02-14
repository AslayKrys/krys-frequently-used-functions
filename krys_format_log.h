#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <pthread.h>
#include <string>
#include <map>
#include <memory>
#include <cppformat/format.h>
#define BOOST_DATE_TIME_SOURCE


#define ENV_LOGPATH "LOGPATH"


/************************************************************
  ---------------------------By Krys---------------------------
input:
a string or char pointer that can be converted to path
description:
create directory recursively
return value:
0 on success, 1 if the directory exists already and -1 
on failure
 ************************************************************/

//递归目录创建, 基于boost


template<bool header_only = true>
class __LOG_DATA__
{
private:
	struct log_struct
	{
		std::unique_ptr<FILE, std::function<void (FILE*)>> log_fp; //智能指针形式的文件指针
		char path[256 + 1]; //储存文件路径的字符串
	};

	static std::map <std::string, struct log_struct> log_map;
#ifdef MULTI_THREAD
	static boost::mutex boost_lock;
#endif
public:
	//日志写入函数
	template<typename ... ARGS>
	static void write_log (const char* type, const char* format, ARGS ... args)
	{
		using namespace std;
		static boost::filesystem::path p = getenv (ENV_LOGPATH);
		p /= type;

		if (boost::filesystem::exists (p))
		{
			boost::filesystem::create_directories (p);
		}

		std::string local_day = boost::gregorian::to_iso_string (boost::gregorian::day_clock::local_day());

		p /= (local_day + ".log");

		auto fp = fopen (p.c_str(), "a+");
		boost::interprocess::file_lock l (p.c_str());
		l.lock();

		fmt::print (fp, format, forward<ARGS>(args)...);

		l.unlock();
		fclose (fp);
	}
};

typedef __LOG_DATA__<true> log_data;

#ifdef MULTI_THREAD
template <bool header_only>
boost::mutex __LOG_DATA__<header_only>::boost_lock;
#endif

template<bool header_only>
std::map<std::string, typename __LOG_DATA__<header_only>::log_struct> __LOG_DATA__<header_only>::log_map;

#define LOG(type,format,...) \
	({\
	 using namespace boost::posix_time;\
	 time_facet *facet = new time_facet ("%H:%M:%S");\
	 std::stringstream ss;\
	 ss.imbue (std::locale (ss.getloc(), facet));\
	 ss << second_clock::local_time ();\
	 ss.str();\
	 log_data::write_log (#type, "{},{},{},{},pid->{}:" format "\n", ss.str().c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__, getpid(), ##__VA_ARGS__);\
	 })

#define LOGRAW(type, format,...) \
	log_data::write_log (#type, format "\n", ##__VA_ARGS__)
