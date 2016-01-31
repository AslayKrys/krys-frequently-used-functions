#ifndef KRYS_H
#define KRYS_H



/*--------------------------------------include system headers-------------------------------------*/
#include <time.h>
#include <memory>
#include <stdio.h>
#include <string>
#include <sys/file.h>
#include <unordered_map>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#define gettid() syscall(__NR_gettid)
/*----------------------------------------------END------------------------------------------------*/



/*---------------------------------------enhaced max macro-----------------------------------------*/
#define MAX(a,b) \
	({\
	__typeof__ (a) A = a;\
	__typeof__ (b) B = b;\
	A>B?A:B;\
	 })
/*----------------------------------------------END------------------------------------------------*/



/*--------------------------------------------file length------------------------------------------*/

#define FILE_LENGTH(fd) \
({unsigned long long current_offset = lseek (fd, 0, SEEK_CUR); unsigned long long file_length = lseek (fd, 0, SEEK_END); lseek (fd, current_offset, SEEK_SET); file_length;})
/*----------------------------------------------END------------------------------------------------*/

/*-------------------------------------------error exit log----------------------------------------*/
#define ERR_EXIT(m,...) do{\
fprintf (stderr, m, ##__VA_ARGS__);\
exit(-1);\
}while(0)
/*----------------------------------------------END------------------------------------------------*/


/*----------------------------------------memory allocation----------------------------------------*/

#define DEFINE_PTR(type,p)\
	type* p = NULL;

#define ALLOC_PTR(p,size) \
do\
{\
	if (p != NULL)\
	{\
		free (p);\
	}\
\
	while (p == NULL)\
	{\
		p = (__typeof__ (p)) malloc (size * sizeof (__typeof__ (*p)));\
	}\
}while (0)

#define FREE(p) do\
{\
	if (p != NULL)\
	{\
		free (p);\
		p = NULL;\
	}\
}while (0)
/*----------------------------------------------END------------------------------------------------*/




/*--------------------------------------modes determination----------------------------------------*/
#define READABLE(path) \
	(\
	 {\
		char*  ___________________________________________________PATH = (path);\
		(___________________________________________________PATH != NULL and\
		 0 == access (___________________________________________________PATH, R_OK)\
		 );\
	 }\
	)

#define WRITABLE(path) \
	(\
	 {\
		char*  ___________________________________________________PATH = (path);\
		(___________________________________________________PATH != NULL and\
		 0 == access (___________________________________________________PATH, W_OK)\
		 );\
	 }\
	)

#define EXECUTABLE(path) \
	(\
	 {\
		char*  ___________________________________________________PATH = (path);\
		(___________________________________________________PATH != NULL and\
		 0 == access (___________________________________________________PATH, X_OK)\
		 );\
	 }\
	)
/*----------------------------------------------END------------------------------------------------*/




/*----------------------------------------type determination---------------------------------------*/
#define ISDIR(path) \
	({\
	 bool ret = false;\
	 do {\
		if (path == NULL)\
		{\
			ret = false;\
			break;\
		}\
		struct stat filestat;\
		if (stat (path, &filestat) < 0)\
		{\
			ret = false;\
			break;\
		}\
		ret = S_ISDIR (filestat.st_mode);\
	 }while (0);\
	 ret;\
	 })

#define ISREG(path) \
	({\
	 int ret = false;\
	 do {\
		if (path == NULL)\
		{\
			ret = false;\
			break;\
		}\
		struct stat filestat;\
		if (stat (path, &filestat) < 0)\
		{\
			ret = false;\
			break;\
		}\
		ret = S_ISREG (filestat.st_mode);\
	 }while (0);\
	 ret;\
	 })
/*----------------------------------------------END------------------------------------------------*/


/*----------------------------------------GNU if extensions----------------------------------------*/
#define iferr(x) if(__builtin_expect(!!(x),0))
#define unlikely(x) if(__builtin_expect(!!(x),0))
#define probably(x) if(__builtin_expect(!!(x),1))
/*----------------------------------------------END------------------------------------------------*/



/*-----------------------------------------C++ simulation------------------------------------------*/
#ifndef __cplusplus
#define or  ||
#define and &&
#define true 1
#define false 0
typedef char bool;
#endif
/*----------------------------------------------END------------------------------------------------*/


/*----------------------------------defining standard terminal IO----------------------------------*/
#define STD_IN 0
#define STD_OUT 1
#define STD_ERR 2
/*----------------------------------------------END------------------------------------------------*/


/*--------------------------------------switch for append mode-------------------------------------*/
#ifdef APPEND 
#define __APPEND O_APPEND
#else
#define __APPEND 0
#endif
/*----------------------------------------------END------------------------------------------------*/

/*-----------------------------------------C++ functions-------------------------------------------*/

std::string load_script (std::string cmd);

/*----------------------------------------------END------------------------------------------------*/
/*----------------------------------extern c for function C++ compatability------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------END------------------------------------------------*/




/*-------------------------------------------IO functions------------------------------------------*/
int set_fl (const int fd, int flags); // set flags
int clr_fl (const int fd, int flags); // clear flags
int redir (int closefile, const char* filepath); // IO redirection

int isdir (const char* ftype);
int isreg (const char* ftype);
int ifexists (const char* ftype);

int r_ok (const char* ftype);
int w_ok (const char* ftype);
int x_ok (const char* ftype);
/*----------------------------------------------END------------------------------------------------*/





/*-------------------------------------process control---------------------------------------------*/
void pr_exit (int status);
void daemonize (const char* work_dir = nullptr);
/*----------------------------------------------END------------------------------------------------*/





/*------------------------------------------log functions------------------------------------------*/
int another_day ();			/*refresh log file on another day*/
int mem_log (int fd, const void* addr, unsigned int len); /*log bitwise data*/

/*----------------------------------------------END------------------------------------------------*/

/*-----------------------------------global variables for logging----------------------------------*/
//extern int current_day;
extern FILE* log_file;
extern int log_fd;
extern char* log_dir;
/*----------------------------------------------END------------------------------------------------*/


/*------------------------------------------end of extern c----------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------END------------------------------------------------*/

#endif /*KRYS_H*/


