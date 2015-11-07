#ifndef KRYSLOG_H
#define KRYSLOG_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libgen.h>
#include <string>
#include <regex>
#include <pthread.h>

typedef struct log_info
{
	int log_fd;
	const char* log_path;
}log_info;

extern log_info main_log;
extern int current_day;

#define KRYS_LOGINIT \
log_info int = 0;
extern log_info main_log;

int get_log_fd (std::string log_title);
std::string get_log_path (std::string log_title);

#define LOG_PRINT(logtype,message,...) do{\
	time_t ntime = time(NULL);\
	struct tm* now = localtime_r (&ntime, NULL);\
	if (now->tm_mday != current_day)\
		another_day (&logtype);\
	dprintf (file_des, "[%02d:%02d:%02d]%s-%s-%d-pid:%06d-tid:%06d:" message "\n", \
				now->tm_hour, now->tm_min, now->tm_sec, __FILE__, __PRETTY_FUNCTION__, __LINE__, getpid(), syscall(__NR_gettid)) \
}while(0)



#endif
