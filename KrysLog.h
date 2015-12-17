#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <pthread.h>
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
	try //如果丢出异常就返回-1并且设置error
	{
		if (!boost::filesystem::exists (dir.parent_path())) //如果父目录不存在则调用自身递归创建
		{
			if (-1 == boost_mkdir (dir.parent_path())) 
			{
				return -1;
			}
		}

		if (!boost::filesystem::exists (dir)) //确保父目录存在的情况下, 如果自身不存在则创建之
		{
			boost::filesystem::create_directory (dir);
		}
		else //如果文件已经存在返回1
		{
			return 1;
		}

	}
	catch (boost::filesystem::filesystem_error err)
	{
		//boost的system_err::code设置的值和标准C中的errno是兼容的
		errno = err.system_error::code ().value();
		return -1;
	}

	return 0; //如果文件不存在并且创建成功, 返回0
}

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
	static int write_log (const char* type, const char* format, ...)
	{
#ifdef MULTI_THREAD
		boost::mutex::scoped_lock lock (boost_lock); //如果在多线程环境下就上锁
#endif

		struct log_struct& log_ref = log_map[type];
		auto temp_fp = log_ref.log_fp.get();

		//如果是第一次使用这个类型的日志(log_map[type]中文件指针为空)则初始化日志文件和文件下然后再写日志

		if (log_ref.log_fp == nullptr)
		{
			init_log (type, log_ref); //日志文件初始化函数
			temp_fp = log_ref.log_fp.get();
		}

		int ret;


		va_list ap;
		va_start (ap, format);

		boost::interprocess::file_lock log_file_lock (log_ref.path); //给文件上锁


		/*-----------------------------------write log and flush buffer------------------------------------*/
		ret = vfprintf (temp_fp, format, ap);
		fflush (temp_fp);
		log_file_lock.unlock();
		/*----------------------------------------------END------------------------------------------------*/

		va_end (ap);

		return ret; //返回写入的字节数
	}

	
	static void init_log (const char* type, struct log_struct& ref, const char* corepath = ".")
	{
		using namespace boost::filesystem;		
		
		const char* log_path = getenv (ENV_LOGPATH); //从环境变量中获取日志的总路径

		path log_path_to_file;

		if (log_path == nullptr) //如果无此环境变量则设置日志的总路径为当前目录下的log文件夹
		{
			log_path_to_file = current_path() / "log";
		}
		else // 如果环境变量存在则把环境变量的值设定为日志的总目录
		{
			log_path_to_file = log_path;
		}

		log_path_to_file /= type; //在日志总目录下创建跟日志类型同名的文件夹

		if (boost_mkdir (log_path_to_file) == -1) //创建该文件夹
		{
			auto up_buffer = std::make_unique<char[]> (128);
			snprintf (up_buffer.get(), 128, "gcore %s -o %d", current_path().string<std::string>().c_str(), getpid());
			system (up_buffer.get()); //留下尸体

			int * a = nullptr; *a = 1; //如果日志目录创建失败则自杀并且留下尸体供开发人员玩耍
		}

		std::string local_day = boost::gregorian::to_iso_string (boost::gregorian::day_clock::local_day());
	
		log_path_to_file /= (local_day + ".log");

		ref.log_fp.reset (fopen (log_path_to_file.string().c_str(), "a+"));

		ref.log_fp.get_deleter() = fclose;
		
		if (ref.log_fp == nullptr)
		{
			auto up_buffer = std::make_unique<char[]> (128);
			snprintf (up_buffer.get(), 128, "gcore %s -o %d", corepath, getpid());
			system (up_buffer.get()); //留下尸体

			const char c = *log_path; //如果日志文件打开失败也自杀, 并且供开发人员玩耍
		}

		strncpy (ref.path, log_path_to_file.string().c_str(), 256);
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
	 log_data::write_log (#type, "%s,%s,%s,%d,pid->%d:" format "\n", ss.str().c_str(), __FILE__, __PRETTY_FUNCTION__, __LINE__, getpid(), ##__VA_ARGS__);\
	 })

#define LOGRAW(type, format,...) \
	log_data::write_log (#type, format "\n", ##__VA_ARGS__)
