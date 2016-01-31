#include <boost/filesystem.hpp>
#include <iostream>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <memory> 
#include <thread>
#include <functional>
#include "third_party_include/fit/partial.hpp"
#include "third_party_include/fit/function.hpp"
#include "third_party_include/fit/infix.hpp"
#include <boost/chrono.hpp>
#include "Krysock.h"

using namespace fit;

#define thread_entry __THREAD_ENTRY__{},

struct __THREAD_ENTRY__
{
	void operator, (const std::function<void ()>&async_func)
	{
		std::thread{async_func}.detach();
	}
};

#define mkdir __MKDIR__{},

struct __MKDIR__
{
	int operator, (const boost::filesystem::path& p)
	{
		boost::system::error_code ec;
		boost::filesystem::create_directories (p, ec);

		if (ec != boost::system::errc::success)
		{
			return -1;
			errno = ec.value();
		}
		return 0;
	}
};

#define sleepfor __SLEEPFOR__{},
struct __SLEEPFOR__
{
	template <class Rep, class Period>
	void operator, (const std::chrono::duration<Rep,Period>& time_span)
	{
		std::this_thread::sleep_for (time_span);
	}
};


#define rm __RM__{},

struct __RM__
{
	int operator, (const boost::filesystem::path& p)
	{
		boost::system::error_code ec;
		boost::filesystem::remove_all (p, ec);

		if (ec != boost::system::errc::success)
		{
			return -1;
			errno = ec.value();
		}
		return 0;
	}
};




#define read_open __FILE_OPEN__{}||
#define write_open __FILE_OPEN__{}&&
#define append_open __FILE_OPEN__{}^
using file_ptr = std::unique_ptr<FILE, std::function<void(FILE*)>>;

struct __FILE_OPEN__
{
	file_ptr operator|| (const char* path)
	{
		return file_ptr (fopen (path, "r"), fclose);
	}
	file_ptr operator&& (const char* path)
	{
		return file_ptr (fopen (path, "w"), fclose);
	}
	file_ptr operator^ (const char* path)
	{
		return file_ptr (fopen (path, "a+"), fclose);
	}
};




#define mv __MV__{}>>

struct __MID_MV__
{
	__MID_MV__ (const boost::filesystem::path& _from):from (_from){}
	int operator>> (const boost::filesystem::path& to)
	{
		boost::system::error_code ec;
		boost::filesystem::rename (from, to, ec);

		if (ec != boost::system::errc::success)
		{
			errno = ec.value();
			return -1;
		}
		return 0;
	}
private:
	boost::filesystem::path from;
};
struct __MV__
{
	__MID_MV__ operator>> (const boost::filesystem::path& from)
	{
		return __MID_MV__ {from};
	}
};




#define ls __LS__{}||

struct __LS__
{
	std::vector<std::string> operator|| (const boost::filesystem::path &p)
	{
		try
		{
			if (!boost::filesystem::is_directory (p)) return {};

			std::vector<std::string> vec_result;

			boost::filesystem::directory_iterator iter_end;
			for (boost::filesystem::directory_iterator iter(p); iter != iter_end; ++iter)
			{
				vec_result.emplace_back (iter->path().filename().string());
			}

			return vec_result;

		}
		catch (...)
		{
			return {};
		}
	}
};




#define tcp_connect __CONN__{}
struct __CONN__
{

};


struct __PORT__;
struct __GET_HOST__
{
	friend struct __PORT__;
	__GET_HOST__ (const char* _host):host(_host){}
private:
	const char* host;
};

struct __HOST__
{
	__GET_HOST__ operator () (__CONN__ , const char* host)const
	{
		return host;
	}
};

FIT_STATIC_FUNCTION(host) = infix_adaptor<__HOST__>();

struct __PORT__
{
	inline int operator() (__GET_HOST__ _host, unsigned short _port)const
	{
		return tcp_open (_host.host, _port);
	}
};


FIT_STATIC_FUNCTION(port) = infix_adaptor<__PORT__>();




#define append_file __FAPPEND_START__{}>>

struct __FAPPEND_MID__ 
{
	int operator>> (std::string filename)
	{
		auto fp = append_open filename.c_str();
		if (fp == nullptr)
		{
			return -1;
		}
		
		return fwrite (this->str.c_str(), 1, this->str.length(), fp.get());
	}
	__FAPPEND_MID__ (const std::string& _str):str (_str){}
	const std::string& str;
};

struct __FAPPEND_START__
{
	__FAPPEND_MID__ operator>> (const std::string& str)
	{
		return str;
	}
};


#define print_string __PRINT__{}>>

struct __PRINT__
{
	void operator>> (const std::string& str_print)
	{
		puts (str_print.c_str());
	}
};

#define touch __TOUCH__{},
struct __TOUCH__
{
	__TOUCH__ operator, (const char* filename)
	{
		try
		{
			if (!boost::filesystem::exists (filename))
			{
				append_open filename;
			}
			else
			{
				boost::filesystem::last_write_time (filename, std::time (nullptr));
			}

		}
		catch (...)
		{

		}
		return (*this);
	}
};


#define Kill __KILL__{}

struct __KILL__
{
	int operator, (int pid)
	{
		return kill (pid, sig);
	}
	__KILL__ operator - (int sig)
	{
		this->sig = sig;
		return (*this);
	}
private:
		int sig = 0;
};


#define quit __QUIT__{},
struct __QUIT__
{
	inline void operator, (int exit_code)
	{
		exit (exit_code);
	}
};
