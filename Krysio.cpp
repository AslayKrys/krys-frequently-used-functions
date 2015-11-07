#define _XOPEN_SOURCE 999999
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "Krys.h"
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "Kryslog.h"

#ifndef __cplusplus
#define or  ||
#define and &&
#endif

// IO redirection
// closefile: fd to be closed
// filepath: file that needs to be redirected to
// RETURN VALUE:returns fd on success and negative values on failure with details logged



int 
redir (int closefile, const char* filepath)
{
	if (NULL == filepath)
	{
		errno = EINVAL;
		return -2;
	}

	int fd = open (filepath, closefile == 0? (O_RDONLY): (O_WRONLY | O_CREAT | __APPEND), 0666);

	if (fd == -1)
	{
		return -1;
	}

	if (closefile > 2 or closefile < 0)
	{
		errno = EINVAL;
		return -1;
	}

	if (dup2 (fd, closefile) == -1)
	{
		return -1;
	}
	return 0;
}

//set fd flags
//fd:file descriptor on which the flag needs to be set
//flags:flag in int 

int
set_fl (int fd, int flags)
{
	int val = -1;

	if ((val = fcntl (fd, F_GETFL, val)) < 0) /*get flags*/
	{
		return -1;
	}

	val |= flags; /*turn on flags*/

	if (fcntl(fd, F_SETFL, val) < 0)
	{
		return -1;
	}

	return 0; 
}


//same pattern as set_fl


int
clr_fl (int fd, int flags)
{
	int val = -1;

	if ((val = fcntl (fd, F_GETFL, val)) < 0)
	{
		return -1;
	}

	val &= ~flags; /*turn off flags*/ 

	if (fcntl(fd, F_SETFL, val) < 0)
	{
		return -1;
	}

	return 0; 
}



int isdir (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	struct stat filestat;
	if (stat (path, &filestat) < 0)
	{
		return 0;
	}

	return S_ISDIR(filestat.st_mode);
}

int ifexists (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	if (0 == access (path, F_OK))
	{
		return 1;
	}

	return 0;
}

int isreg (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	struct stat filestat;
	if (stat (path, &filestat) < 0)
	{
		return 0;
	}

	return S_ISREG(filestat.st_mode);
}

int r_ok (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	if (0 == access (path, R_OK))
	{
		return 1;
	}

	return 0;
}

int w_ok (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	if (0 == access (path, W_OK))
	{
		return 1;
	}

	return 0;
}

int x_ok (const char* path)
{
	if (NULL == path)
	{
		return 0;
	}

	if (0 == access (path, X_OK))
	{
		return 1;
	}

	return 0;
}

#define LOG_WIDTH 16

#define PRINTSPACES(fd) \
	dprintf (fd, "     ")

#define ENDLINESPACES(fd) \
	dprintf (fd, "     \n")


int mem_log (int fd, const void* addr, unsigned int len)
{

	if (fd < 0 or addr == NULL)
	{
		return -1;
	}

	char* end = (char*)addr + len;
	char* pos = (char*)addr;
	DEFINE_PTR (char, charbuf);
	ALLOC_PTR (charbuf, LOG_WIDTH);


	//header
	
	dprintf (fd, "-----------------------------------------------------------------------------\n");

	//PRINTSPACES (fd);

	dprintf (fd, "Lenth of the message:%u (HEX 0X%X)byte(s).", len, len);
	
	dprintf (fd, "\n\n");


	//regular lines
	while (pos + LOG_WIDTH <= end)
	{
		//PRINTSPACES (fd);

		dprintf (fd, "%04X     |%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX|", (unsigned int)(pos - (char*)addr), 
		(unsigned char)*pos, (unsigned char)*(pos + 1), (unsigned char)*(pos + 2), (unsigned char)*(pos + 3),
		(unsigned char)*(pos + 4), (unsigned char)*(pos + 5), (unsigned char)*(pos + 6), (unsigned char)*(pos + 7), 
		(unsigned char)*(pos + 8), (unsigned char)*(pos + 9), (unsigned char)*(pos + 10), (unsigned char)*(pos + 11), 
		(unsigned char)*(pos + 12), (unsigned char)*(pos + 13), (unsigned char)*(pos + 14), (unsigned char)*(pos + 15));
		
		PRINTSPACES (fd);

		memcpy (charbuf, pos, LOG_WIDTH);

		for (char* iter = charbuf; iter != charbuf + LOG_WIDTH; iter++)
		{
			if (*iter == '\n' or *iter == '\t')
			{
				*iter = ' ';
			}
		}

		dprintf (fd, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
		*charbuf, *(charbuf + 1), *(charbuf + 2), *(charbuf + 3), *(charbuf + 4), *(charbuf + 5), *(charbuf + 6), *(charbuf + 7), *(charbuf + 8),
		*(charbuf + 9), *(charbuf + 10), *(charbuf + 11), *(charbuf + 12), *(charbuf + 13), *(charbuf + 14), *(charbuf + 15));

		ENDLINESPACES (fd);

		pos += LOG_WIDTH;

	}

	//last line
	
	if (pos != end)
	{

		//PRINTSPACES (fd);
		dprintf (fd, "%04X     |", (unsigned int)(pos - (char*)addr));

		for (char* line_iterator = pos; line_iterator != end; line_iterator++)
		{
			dprintf (fd, "%02hhX ", (unsigned char)*line_iterator);
		}

		char space[48] = {0};

		memset (space, ' ', (16 - (end - pos))*3 - 1);

		dprintf (fd, "%s", space);



		dprintf (fd, "|");
		PRINTSPACES (fd);

		for (char* line_iterator = pos; line_iterator != end; line_iterator++)
		{
			dprintf (fd, "%c", *line_iterator == '\n' or *line_iterator == '\t' ? ' ' : *line_iterator);
		}

		memset (space, 0x00, sizeof (space));
		dprintf (fd, "\n");
	}

	dprintf (fd, "\n");

	//PRINTSPACES (fd);
	dprintf (fd, "End of message." "\n-----------------------------------------------------------------------------\n");
	

	FREE (charbuf);
	return 0;
}


