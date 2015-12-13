#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <map>
#include <memory>
#define BOOST_DATE_TIME_SOURCE

using namespace std;

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

inline int boost_mkdir (const boost::filesystem::path& dir)
{
	try
	{
		if (!boost::filesystem::exists (dir.parent_path()))
		{
			if (-1 == boost_mkdir (dir.parent_path()))
			{
				return -1;
			}

		}

		if (!boost::filesystem::exists (dir))
		{
			boost::filesystem::create_directory (dir);
		}

		else
		{
			return 1;
		}

	}
	catch (boost::filesystem::filesystem_error err)
	{
		errno = err.system_error::code ().value();
		return -1;
	}

	return 0;

}

template<bool header_only = true>
class __LOG_DATA__
{
private:
	struct log_struct
	{
		std::unique_ptr<FILE, std::function<void (FILE*)>> log_fp;
		char path[256 + 1];
	};

	static std::map <std::string, struct log_struct> log_map;
public:
	//日志写入函数
	static int write_log (const char* type, const char* format, ...)
	{
		
		struct log_struct& log_ref = log_map[type];
		if (log_ref.log_fp == nullptr)
		{
			init_log (type, log_ref);
		}

		int ret;
		boost::interprocess::file_lock lock (log_ref.path);


		va_list ap;
		va_start (ap, format);

		ret = vfprintf (log_ref.log_fp.get(), format, ap);

		va_end (ap);

		return ret;
	}


	
	static void init_log (const char* type, struct log_struct& ref)
	{
		using namespace boost::filesystem;		

		const char* log_path = getenv (ENV_LOGPATH);

		path log_path_to_file (log_path);

		log_path_to_file /= type;

		boost_mkdir (log_path_to_file);

		std::string local_day = boost::gregorian::to_iso_string (boost::gregorian::day_clock::local_day());
	
		cout << local_day << endl;

		log_path_to_file /= (local_day + ".log");

		cout << log_path_to_file.c_str() << endl;

		ref.log_fp.reset (fopen (log_path_to_file.c_str(), "a+"));

		ref.log_fp.get_deleter() = fclose;
		
		if (ref.log_fp == nullptr)
		{
			throw std::string ("log init failed");
		}

		strncpy (ref.path, log_path_to_file.c_str(), 256);
	}
};

typedef __LOG_DATA__<true> log_data;

template<bool header_only>
std::map<std::string, typename __LOG_DATA__<header_only>::log_struct> __LOG_DATA__<header_only>::log_map;

#define LOG(type,format,...) \
	log_data::write_log (#type, "%s,%s,%d,pid->%d:" format "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, getpid(), ##__VA_ARGS__)
