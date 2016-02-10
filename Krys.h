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





int mem_log (int fd, const void* addr, unsigned int len); /*log bitwise data*/



#endif /*KRYS_H*/


