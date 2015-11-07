#define   _POSIX_SOURCE
#include "Kryslog.h"
#include "Krys.h"
#include <regex>

struct tm*  get_time ();
int current_day = 0;


FILE* log_file = NULL;


int log_fd = -1;
char* log_dir = NULL;


typedef struct 
{
	FILE* log_ptr;
	char* env_name;
	char* prefix;
	char* postfix;
}log_types;

log_info main_log = {-1, "."};

static pthread_mutex_t day_mutex = PTHREAD_MUTEX_INITIALIZER;

int another_day ()
{
	// Thread Lock.
	pthread_mutex_lock (&day_mutex);

	/*Obtaining the time for now. */

	time_t time_int = time (NULL);
	struct tm* now = localtime (&time_int);

	/*if the file pointer is already allocated,close the pointer for reallocation.*/

	if (log_file != NULL)
	{
		fclose (log_file);
		log_file = NULL;
	}

	/*----------------reallocation--------------------------------------*/

	/*setting the path.*/
	char log_path [256] = {0};

	const char* LOGDIR = getenv ("LOGDIR");

	if (LOGDIR != NULL)
	{

		strcat (log_path, LOGDIR);

		if (log_path[strlen (log_path) - 1] != '/')
		{
			strcat (log_path, "/");
		}
	}

	else
	{
		strcpy (log_path, "./");
	}

	char date[32] = {0};
	
	if (now == NULL)
	{
		exit (-76);
	}


	sprintf (date, "%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday);

	strcat (log_path, date);
	strcat (log_path, ".log");


	log_file = fopen (log_path, "a+");
	
	/*trying local path if reallocation fails.*/

	if (log_file == NULL)
	{
		log_file = fopen (basename (log_path), "a+");
	}

	/*exiting on double failure.*/

	if (log_file == NULL)
	{
		exit (EXIT_FAILURE);
	}

	/*----------------end of reallocation------------------------------------------*/

	log_fd = fileno (log_file);

	/*setting current day. */

	current_day = now->tm_mday;

	/*releasing thread lock.*/

	pthread_mutex_unlock (&day_mutex);

	return 0;
}

int get_log_fd (std::string log_title)
{
	std::string log_config = getenv ("LOGCONFIG");
	std::string config_line;

	std::ifstream read_config;

	read_config.open (log_config); 
	if (read_config.fail()) exit (EXIT_FAILURE);
	std::regex title (std::string (R"([[:space:]]*\[)") + log_title + R"(\][[:space:]]*)");
	std::regex next_title (R"([[:space:]]*\[.*\][[:space:]]*)");

	std::regex fd_line (R"(fd[[:space:]]*=[[:space:]]*([[:digit:]]+))");
	std::smatch fd_match;

	while (!read_config.eof())
	{
		std::getline (read_config, config_line);

		if (!std::regex_match (config_line, title))
		{
			continue;
		}
		else
		{
			while (!read_config.eof())
			{
				std::getline (read_config, config_line);
				if (std::regex_match (config_line, next_title))
				{
					break;
				}

				if (std::regex_search (config_line, fd_match, fd_line))
				{
					return atoi (fd_match[1].str().c_str());
				}
			}
			break;
		}

	}

	return -1;
}


std::string get_log_path (std::string log_title)
{
	std::string log_config = getenv ("LOGCONFIG");
	std::string config_line;

	std::ifstream read_config;

	read_config.open (log_config); 
	if (read_config.fail()) exit (EXIT_FAILURE);
	std::regex title (std::string (R"([[:space:]]*\[)") + log_title + R"(\][[:space:]]*)");
	std::regex next_title (R"([[:space:]]*\[.*\][[:space:]]*)");

	std::regex path_line (R"(^[[:space:]]*path[[:space:]]*=[[:space:]]*([^[:space:]]+)[[:space:]]*$)");
	std::smatch path_match;
	std::string path_str;

	
	std::smatch env_match;
	std::regex env_pattern (R"((\$[[:alnum:]]+))");

	while (!read_config.eof())
	{
		std::getline (read_config, config_line);

		if (!std::regex_match (config_line, title))
		{
			continue;
		}
		else
		{
			while (!read_config.eof())
			{
				std::getline (read_config, config_line);
				if (std::regex_match (config_line, next_title))
				{
					break;
				}

				if (std::regex_search (config_line, path_match, path_line))
				{
					path_str = path_match[1].str();

					while (std::regex_search (path_str, env_match, env_pattern))
					{
						path_str.replace (path_str.find_first_of(env_match[1].str(), 0),
							env_match[1].length(),
							getenv (env_match[1].str().c_str() + 1) != nullptr ? 
							getenv (env_match[1].str().c_str() + 1) : "");
					}
					return path_str;
				}
			}
			break;
		}

	}

	return std::string ("");
}

