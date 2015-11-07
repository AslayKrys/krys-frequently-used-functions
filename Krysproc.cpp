#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "Krys.h"

#define PRINT(m,...)\
	printf (m "\n", ##__VA_ARGS__)

void
pr_exit (int status)
{
	if (WIFEXITED (status))
	{
		PRINT ("normal termination with exit status:%d", WEXITSTATUS (status));
	}
	else if (WIFSIGNALED (status))
	{
		PRINT ("abnormal termination with signal number = %d%s", WTERMSIG (status),
#ifdef WCOREDUMP	
		WCOREDUMP (status) ? "(core dumped)":				
#endif
		"");
	}
	else if (WIFSTOPPED (status))
	{
		PRINT ("child stopped with signal number:%d", WSTOPSIG (status));
	}
}

void 
daemonize (const char* work_dir)
{
	pid_t pid = fork ();
	if (pid == -1)
	{
		exit (EXIT_FAILURE);
	}

	if (pid > 0)
	{
		exit (0);
	}

	setsid ();

	if (work_dir != NULL)
	{
		chdir (work_dir);
	}

	umask (0);
	
	for (int i=0; i<1024; i++)
	{
		close (i);
	}
}

std::string load_script (std::string cmd)
{
	int tmp_stdout, tmp_stdin;
	int fifo[2];
	int len;
	std::string result;



	tmp_stdout = dup (1);
	tmp_stdin = dup (0);
	close (0);
	close (1);

	if (pipe (fifo) == -1)
	{
		return "";
	}

	system (cmd.c_str());

	auto buffer = std::make_unique<char[]> (1024 + 1);

	close (1);

	while (1)
	{
		len = read (0, buffer.get(), 1024);
		if (len == 0 or len == -1)
		{
			break;
		}

		buffer[len] = '\0';

		result += buffer.get();
	}

	close (0);

	dup2 (tmp_stdin, 0); close (tmp_stdin);
	dup2 (tmp_stdout, 1); close (tmp_stdout); 

	len = result.length();

	result = result.substr (0, len - 1);

	return result;
}
