#include <cstdio>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cppformat/format.h>

class file_ptr
{
public:
	file_ptr (const char* path, const char* mode)
	{
		this->fp = ::fopen (path, mode);
	}

	file_ptr (const file_ptr&) = delete;

	file_ptr (file_ptr&& that)
	{
		this->fp = that.fp;
		that.fp = nullptr;
	}

	auto get () const
	{
		return this->fp;
	}

	auto reset (FILE* new_fp)
	{
		this->close();
		this->fp = new_fp;
	}

	template<typename ...ARGS>
	int format_print (const char* fmt, ARGS...args)
	{
		fmt::print (this->fp, fmt, std::forward<ARGS>(args)...);
	}

	FILE* release ()
	{
		FILE* old_fp = this->fp;
		this->fp = nullptr;
		return old_fp;
	}

	auto operator == (void* ptr) const
	{
		return this->fp == ptr;
	}

	void operator = (const file_ptr&that) = delete;
	void operator = (const FILE*) = delete;

	template<class...ARGS>
	int fprintf (const char* fmt, ARGS...args)
   	{
   		return ::fprintf (this->fp, fmt, std::forward<ARGS>(args)...);
   	}

	void close ()
	{
		if (fp != nullptr)
		{
			::fclose (fp), fp = nullptr;
		}
	}

	auto operator = (file_ptr&& that)
	{
		this->close ();
		this->fp = that.fp;
		that.fp = nullptr;
	}

	compl file_ptr ()
	{
		this->close();
	}
private:
	FILE* fp;

protected:
}; 

class file_descriptor
{
public:
	file_descriptor (int _fd):fd (_fd) { }

	void operator = (int) = delete;
	void operator = (file_descriptor) = delete;

	void operator = (file_descriptor&& that)
	{
		this->fd = that.fd;
		that.fd = -1;
	}

	int dup ()
	{
		return ::dup (fd);
	}
	
	int dup2 (int new_fd)
	{
		return ::dup2 (this->fd, new_fd);
	}

	template<typename ... ARGS>
	int dprintf (const char* fmt, ARGS ... args)
	{
		return ::dprintf (this->fd, fmt, std::forward<ARGS>(args)...);
	}

	compl file_descriptor ()
	{
		if (this->fd >= 0)
		{
			close (fd);
		}
	}
private:
	int fd;
protected:
};
