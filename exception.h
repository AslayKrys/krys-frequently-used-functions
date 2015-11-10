#include <exception>
#include <execinfo.h>
#include <string>
#include <stdlib.h>

namespace krys
{
	class exception;
}

class exception : public std::exception
{
public:
	exception ()
	{
		fill_backtrace ();
	}
	compl exception () {}

	const char* backtrace ()
	{
		return call_stack.c_str();
	}

private:

	void fill_backtrace ()
	{
		const int len = 200;
		void* backtrace_buffer[len];

		int nptrs = ::backtrace (backtrace_buffer, len);

		char** strings = ::backtrace_symbols (backtrace_buffer, nptrs);

		if (strings != nullptr)
		{
			for (int i=0; i<nptrs; ++i)
			{
				call_stack.append (strings[i]);
				call_stack.push_back ('\n');
			}

			free (strings);
		}
	}

private:
	std::string call_stack;

protected:
};
