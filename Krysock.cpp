#include "Krysock.h"
#include "Krys.h"
#include <memory>
#include <utility>
#include <netinet/tcp.h>

/************************************************************
---------------------------By Krys---------------------------
input:
	_addrstr address string which can be hostname or 
	IP address
	
	_addr a 32 bit output value
description:
	convert a hostname or string IP address to a standard
	4 bytes address
return value:
	0 on success and -1 on failure with errno 
	properly set
************************************************************/

int
analysis_addr (const char *_addrstr, u_int32_t *_addr)
{
    struct hostent *hent; 

	/*--------------------------------trying for IP address--------------------------------------------*/
    if ((*_addr = inet_addr (_addrstr)) != INADDR_NONE)
	{
        return 0;
	}
	/*----------------------------------------------END------------------------------------------------*/
  
	/*---------------------------------trying for hostname---------------------------------------------*/
    if ((hent = gethostbyname (_addrstr)) != NULL ) 
	{
        *_addr = ((struct in_addr *)(hent->h_addr_list <:0:>))->s_addr;
        return 0;
    }
	/*----------------------------------------------END------------------------------------------------*/


	/*-------------------------------------returning on failure----------------------------------------*/
	errno = EINVAL;
    return -1;
	/*----------------------------------------------END------------------------------------------------*/
}


int 
udp_bind (const char* ip, unsigned short port)
{

	if (ip == NULL or port == 0)
	{
		errno = EINVAL;
		return -1;
	}

	int udp_socket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (udp_socket == -1)
	{
		return -1;
	}

	struct sockaddr_in srv_addr;

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons (port);

	if (bind (udp_socket, (struct sockaddr*)&srv_addr, sizeof (srv_addr)) == -1)
	{
		return -1;
	}

	return udp_socket;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  read_timeout
 *  Description:  This funciton accepts a socket_ file descriptor and a time_out value in 
 *  seconds.
 *  RETURN VALUE: On success 0,and -1 on failure. If the failure is caused by time_out 
 *  errno is set to ETIMEDOUT.
 * =====================================================================================
 */


int 
read_timeout (int socket_, unsigned int sec)
{
	int ret = 0;

	if (sec > 0)
	{

		/*----------------------------registering fd and timeout value to select---------------------------*/
		fd_set read_fdset;
		struct timeval time_out;

		FD_ZERO (&read_fdset); /*initialize fd set*/
		FD_SET (socket_, &read_fdset); /*registering socket*/

		time_out.tv_sec = sec; /*set timeout value*/
		time_out.tv_usec = 0;
		/*----------------------------------------------END------------------------------------------------*/



		/*-------------------------------------------timeout test------------------------------------------*/
		do 
		{
			ret = select (socket_ + 1, &read_fdset, NULL, NULL, &time_out); /*select is interested in read event only*/
		} while (ret < 0 and errno == EINTR);
		/*----------------------------------------------END------------------------------------------------*/



		/*--------------------------------------determine success or failure-------------------------------*/
		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret == 1)
		{
			ret = 0;
		}
		/*----------------------------------------------END------------------------------------------------*/
	}
	
	return ret; /*return*/

}		/* -----  end of function read_timeout  ----- */

/************************************************************
---------------------------By Krys---------------------------
input:
	socket_ --> socket for tcp transfer
	buf --> buffer that data will be read into
	len --> length of bytes
description:
	this function peeks data in system buffer without
	retrieving
return value:
	actual length peeked on success and -1 on failure
************************************************************/

int 
recv_peek (int socket_, void* buf, size_t len)
{
	int recv_len = 0;

	while ((recv_len = recv (socket_, buf, len, MSG_PEEK)) == -1 and errno == EINTR);

	return recv_len;
}



ssize_t 
tcp_readline (int socket_, void* buf, int max_len)
{
	int32_t readval = 0, offset = 0; char* tmp_ptr = (char*)buf; 


	while (1) /*read loop*/
	{

		/*----------------------------------------peek buffer----------------------------------------------*/
		readval = recv_peek (socket_, tmp_ptr, max_len - offset);

		iferr (readval <= 0)
		{
			return readval;
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*------------------------------------------examine buffer-----------------------------------------*/
		for (int32_t i=0; i<readval; i++)
		{
			if (tmp_ptr <:i:> == '\n')
			{
				readval = tcp_read (socket_, tmp_ptr, i + 1);

				return readval == i + 1 ? readval + offset : -1;
			}
		}
		/*----------------------------------------------END------------------------------------------------*/


		/*--------------------------------end flag not found, updating offset------------------------------*/
		if (readval + offset > max_len)
		{
			errno = EIO;
			return -1;
		}

		offset += readval;

		if ((readval = tcp_read (socket_, tmp_ptr, offset)) != offset)
		{
			return -1;
		}

		tmp_ptr += offset;
		/*----------------------------------------------END------------------------------------------------*/

	}


	return -1;
}



/************************************************************
---------------------------By Krys---------------------------
input:
	socket_ --> socket fd for tcp connection
	buffer --> data to be read into
	len --> expected length to be read
description:
	tcp_read performs a read loop to make sure that given 
	bytes are read before returning however the return 
	value might still be less than length given due to
	the connection condition or the opposite side sends 
	a terminating request
return value:
	actual bytes read (normally the length givei)
	and -1 on system error
************************************************************/


int 
tcp_read (int socket_, void* buf, int len)
{
	int32_t offset = 0, readval = 0;	/*offset and bytes that has been read*/
	char* tmp_ptr = (char*)buf;			/*convert buf to char**/

	while (len > offset) /*read loop*/
	{
		/*-------------------------------execution of the system API for reading---------------------------*/
		iferr ((readval = read (socket_, tmp_ptr + offset, len - offset)) <= 0)
		{
			iferr (readval == -1 and errno == EINTR)
			{
				continue;
			}
			if (readval == 0) errno = ECONNABORTED;
			break;
		}
		/*----------------------------------------------END------------------------------------------------*/

		offset += readval; /*updating offset*/

	}

	return offset;
}


int 
tcp_read_timeout (int socket_, void* buf, int len, int second)
{
	/*--------------------------------------parameter checking-----------------------------------------*/
	if (second < 0 or len <=0)
	{
		errno = EINVAL;
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/


	/*-------------------------------------variable definitions----------------------------------------*/
	int32_t offset = 0, readval = 0; char* tmp_ptr = (char*)buf; 
	struct timeval time_out;
	fd_set read_fdset;
	int select_return;
	/*----------------------------------------------END------------------------------------------------*/


	/*----------------------------registering fd and timeout value to select---------------------------*/
	FD_ZERO (&read_fdset); /*initialize fd set*/
	FD_SET (socket_, &read_fdset); /*registering socket*/
	/*----------------------------------------------END------------------------------------------------*/

	while (len > offset)
	{

		/*---------if there is no data in the buffer for a period of time, then the function returns-------*/
		time_out.tv_sec = second;   
		time_out.tv_usec = 0;

		do 
		{
			select_return = select (socket_ + 1, &read_fdset, NULL, NULL, &time_out); /*select is interested in read event only*/
		} while (select_return < 0 and errno == EINTR);

		if (select_return == 0)
		{
			errno = ETIMEDOUT;
			break;
		}
		/*----------------------------------------------END------------------------------------------------*/




		/*----------------execution of the system API for reading after a timeout examing------------------*/
		iferr ((readval = read (socket_, tmp_ptr + offset, len - offset)) <= 0)
		{
			iferr (readval == -1 and errno == EINTR)
			{
				continue;
			}
			if (readval == 0) errno = ECONNABORTED;
			break;
		}
		/*----------------------------------------------END------------------------------------------------*/

		offset += readval; /*update offset*/
	}

	return offset;				/*return offset value*/
}


int
krys_read (int socket_, void** buf)
{

	/*----------------------------------if buf is null, return error-----------------------------------*/
	if (buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/



	/*----------------------------------------handling meta data---------------------------------------*/
	int len;

	iferr (tcp_read_timeout (socket_, &len, sizeof len, 10) != sizeof len)
	{
		*buf = NULL;
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/




	/*-------------------------------------------handling data-----------------------------------------*/
	len = ntohl (len); /*convert len to network bit sequence*/

	//*buf = (__typeof__ (*buf))ALLOC_MEM (len); /*alloc memory with given size*/
	*buf = malloc (len); /*alloc memory with given size*/

	iferr (tcp_read_timeout (socket_, *buf, len, 10) != len)    /*read data*/
	{
		FREE (*buf);
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	return len;
}


int
krys_write (int socket_, const void* buf, int len)
{
	/*-------------------------------------validating parameters---------------------------------------*/
	iferr (len <= 0 or buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	int htonl_len = htonl (len);	/*convert length to net byte sequence*/

	/*--------------------------------------write message header---------------------------------------*/
	iferr (tcp_write (socket_, &htonl_len, sizeof len) != sizeof htonl_len)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	/*---------------------------------------write message body----------------------------------------*/
	iferr (tcp_write (socket_, buf, len) != len)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	return len; /*return length*/
	
}


int 
tcp_write (int socket_, const void* buf, int len)
{
	int32_t offset = 0, write_val = 0;


	while (len > offset)
	{
		iferr ((write_val = write (socket_, (char*)buf + offset, len - offset)) == -1)
		{
			if (errno != EINTR) break;

			continue;
		}

		offset += write_val;

	}

	return offset;
}



/*tcp_open: opening a connection to a tcp server with given hostname/address and port number*/

int 
tcp_open (const char* host, unsigned short port)
{

	/*---------------------------------------validating parameters-------------------------------------*/
	if (host == NULL or port == 0)
	{
		errno = EINVAL;
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/




	/*---------------------------------------variable definitions--------------------------------------*/
	int srvfd = -1;						/*server socket*/
	u_int32_t uint32_addr = 0;			/*integer IP address*/
	struct sockaddr_in srvaddr;			/*server address*/
	/*----------------------------------------------END------------------------------------------------*/




	/*---------------------------------------analysing hostname----------------------------------------*/
	if (analysis_addr (host, &uint32_addr) == -1)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/




	/*------------------------------------------initializing socket------------------------------------*/
	srvfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (srvfd == -1)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/



	/*---------------------------------------setting server address------------------------------------*/
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons (port);
	srvaddr.sin_addr.s_addr = uint32_addr;
	/*----------------------------------------------END------------------------------------------------*/



	/*-------------------------------------proceeding with connection----------------------------------*/
	if (connect (srvfd, (struct sockaddr*)&srvaddr, sizeof (srvaddr)) < 0)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	return srvfd; /*returning file descriptor*/
}








// Function: tcp_listen
// INPUT:ip in C string type , port in unsigned short
// RETURN VALUE: On success,the function returns the file descriptor of the listening socket_ 
// and -1 is returned if any system error occurs.

int 
tcp_listen (const char* host, unsigned short port)
{

	/*-----------------------------------------variable definitions------------------------------------*/
	int listenfd = -1;
	int sockstat = 1;

	struct sockaddr_in srvaddr; 
	srvaddr.sin_family = AF_INET;						/*value setting*/
	srvaddr.sin_port = htons (port);					/*port*/

	if (analysis_addr (host, &srvaddr.sin_addr.s_addr) == -1)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/


	signal (SIGPIPE, SIG_IGN);			/*ignore the signal of broken pipe*/


	/*------------------------------------------creating tcp socket------------------------------------*/
	listenfd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP);	

	if (listenfd == -1)									
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/




	/*------------------------set socket options and bind it to a specific IP/PORT---------------------*/
	if (setsockopt (listenfd, SOL_SOCKET, SO_REUSEADDR, &sockstat, sizeof (sockstat)) < 0) /*setting reuse*/
	{
		return -1;
	}


	if (bind (listenfd, (__CONST_SOCKADDR_ARG)&srvaddr, sizeof (srvaddr)) < 0) /*bind ip and port to a socket_*/
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/



	/*------------------------------------listening on specific IP/PORT--------------------------------*/
	if (listen (listenfd, SOMAXCONN) < 0)				
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	return listenfd; /*return the socket file descriptor*/

}






// Function: tcp_accept
// INPUT:socket_ file descriptor.
// RETURN VALUE: On success,the function returns the file descriptor of the connected socked
// and -1 is returned if any system error occurs.


static __thread	struct sockaddr_in peeraddr;
static __thread socklen_t peerlen = sizeof (peeraddr);

static __thread struct client_info peer;


client_info* 
tcp_accept (int socket_)
{
	peer.client_fd = accept (socket_, (struct sockaddr*)&peeraddr, &peerlen);

	if (peer.client_fd != -1)
	{
		strcpy (peer.client_address, inet_ntoa (peeraddr.sin_addr));
		peer.client_port = ntohs (peeraddr.sin_port);
	}

	return &peer;
}





// Function: socket_ip
// INPUT:socket_ file descriptor.
// RETURN VALUE: ip in C string on success,and NULL on error.




char* 
socket_ip (int socket_)
{
	struct sockaddr_in peeraddr__;

	socklen_t peerlen__ = sizeof (peeraddr__);


	if (getpeername (socket_, (__SOCKADDR_ARG)&peeraddr__, &peerlen__) == -1)
	{
		perror ("error at getpeername.");
		return NULL;
	}

	return inet_ntoa (peeraddr__.sin_addr);
}



// Function: socket_port
// INPUT:socket_ file descriptor.
// RETURN VALUE: positive port number in unsigned short on success,
// and 0 on error.

unsigned short 
socket_port (int socket_)
{
	struct sockaddr_in peeraddr__;

	socklen_t peerlen__ = sizeof (peeraddr__);

	

	if (getpeername (socket_, (__SOCKADDR_ARG)&peeraddr__, &peerlen__) == -1)
	{
		perror ("error at getpeername.");
		return 0;
	}

	return ntohs (peeraddr__.sin_port);
}

typedef struct aaa {
   int n;
   short s;
}aaa;

int
java_read (int socket_, std::unique_ptr<char[]>& buf, int time_out)
{
	unsigned short len;
	/*----------------------------------------handling meta data---------------------------------------*/

	iferr (tcp_read_timeout (socket_, &len, sizeof len, time_out) not_eq sizeof len)
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	/*-------------------------------------------handling data-----------------------------------------*/
	len = ntohs (len); /*convert len to network bit sequence*/

	buf = std::make_unique<char[]> (len + 1);

	iferr (tcp_read_timeout (socket_, buf.get(), len, time_out) != (signed int)len)    /*read data*/
	{
		return -1;
	}
	/*----------------------------------------------END------------------------------------------------*/

	buf[len] = '\0';

	return len;
}


int
java_write (int socket_, const void* buf, unsigned short len)
{
	static __thread char write_buffer [65538];

	unsigned short htons_len = htons (len);
	iferr (len <= 0 or buf == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	memcpy (write_buffer, &htons_len, sizeof htons_len);

	memcpy (write_buffer + sizeof len, buf, len);

	iferr (tcp_write (socket_, write_buffer, len + sizeof htons_len) not_eq (int)(len + sizeof htons_len))
	{
		return -1;
	}

	return len;

}


std::string
string_receive (int socket_, int timeout)
{
	u_int32_t len;
	std::unique_ptr<char[]> tempstr; 

	/*----------------------------------------handling meta data---------------------------------------*/

	iferr (tcp_read_timeout (socket_, &len, sizeof len, timeout) not_eq sizeof len)
	{
		return std::string ("");
	}
	/*----------------------------------------------END------------------------------------------------*/

	/*-------------------------------------------handling data-----------------------------------------*/
	len = ntohl (len); /*convert len to network bit sequence*/


	tempstr = std::make_unique<char<::>> (len + 1);

	iferr (tcp_read_timeout (socket_, tempstr.get(), len, timeout) != (int)len)    /*read data*/
	{
		return std::string ("");
	}
	/*----------------------------------------------END------------------------------------------------*/
	tempstr<:len:> = '\0';

	return std::string (tempstr.get());
}


bool string_send (int socket_, const std::string& str)
{
	int32_t htonl_len = htonl ((long)str.length());

	return socket_ < 0 or str.length() == 0 ? 
		false: tcp_write (socket_, &htonl_len, sizeof htonl_len) != sizeof htonl_len ?
		false: tcp_write (socket_, str.c_str(), str.length()) != (int)str.length() ? 
		false: true;
}




int send_fd (int sock, int fd_sent)
{
	int ret;
	struct msghdr msg;
	struct cmsghdr* p_cmsg = NULL;
	struct iovec vec;
	char cmsgbuf [CMSG_SPACE(sizeof fd_sent)];
	int* p_fds;
	char sendchar = 0;

	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof cmsgbuf;
	p_cmsg = CMSG_FIRSTHDR (&msg);
	p_cmsg->cmsg_level = SOL_SOCKET;
	p_cmsg->cmsg_type = SCM_RIGHTS;
	p_cmsg->cmsg_len = CMSG_LEN (sizeof fd_sent);
	p_fds = (int*)CMSG_DATA (p_cmsg);
	*p_fds = fd_sent;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;

	vec.iov_base = &sendchar;
	vec.iov_len = sizeof sendchar;

	ret = sendmsg (sock, &msg, 0);
	if (ret == -1)
	{
		return -1;
	}

	return 0;
}



int recv_fd (const int sock_fd)
{
	int ret;
	struct msghdr msg;
	char recvchar;
	struct iovec vec;
	int recv_fd;
	char cmsgbuf<:CMSG_SPACE(sizeof recv_fd):>;
	struct cmsghdr *p_cmsg = NULL;
	int* p_fd;
	vec.iov_base = &recvchar;
	vec.iov_len = sizeof recvchar;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof cmsgbuf;
	msg.msg_flags = 0;

	p_fd = (int*)CMSG_DATA (CMSG_FIRSTHDR (&msg));
	*p_fd = -1;

	ret = recvmsg (sock_fd, &msg, 0);
	if (ret != 1)
	{
		return -1;
	}

	p_cmsg = CMSG_FIRSTHDR (&msg);
	if (p_cmsg == NULL)
	{
		return -1;
	}

	p_fd = (int*)CMSG_DATA (p_cmsg);
	recv_fd = *p_fd;

	if (recv_fd == -1)
	{
		return -1;
	}

	return recv_fd;
}

int fd_obtain (int fd, const char* socket_file)
{
	close (fd);

	/*------------------------------------defining variables-------------------------------------------*/
	int unix_domain_socket;

	struct sockaddr_un unix_domain_address;								/*struct that holds the address for unix domain IPC*/
	unix_domain_address.sun_family = PF_UNIX;							/*set protocol family as PF_UNIX*/
	strcpy (unix_domain_address.sun_path, socket_file);				/*set socket file path*/

	/*----------------------------------------------END------------------------------------------------*/




	/*-----------------registering unix domain socket and establish IPC connection---------------------*/
	unix_domain_socket = socket (PF_UNIX, SOCK_STREAM, 0);				/*socket registration*/
	if (unix_domain_socket == -1)
	{
		exit (EXIT_FAILURE);
	}


	while (connect (unix_domain_socket,									/*connecting*/
					(struct sockaddr*)&unix_domain_address, sizeof unix_domain_address) == -1) 
	{
		if (errno == ECONNREFUSED or errno == EINTR)
		{
			sleep (1);
			continue;
		}

		exit (EXIT_FAILURE);
	}
	/*-------------------------------------------------------------------------------------------------*/




	/*------------obtain tcp socket via unix domain IPC mechanism--------------------------------------*/

	tcp_write (unix_domain_socket, &fd, sizeof fd);


	return recv_fd (unix_domain_socket);
	/*----------------------------------------------END------------------------------------------------*/
}


int unlimit_fd (int max)
{
	struct rlimit rl;

	rl.rlim_cur = max;
	rl.rlim_max = max;

	return setrlimit (RLIMIT_NOFILE, &rl);
}

