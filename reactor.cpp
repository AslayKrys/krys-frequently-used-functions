#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include "Krysock.h"
#include <iostream>
#include <vector>
#include <memory>
#include "Krysio.h"

int reactor_listen (const char* host, unsigned short port)
{
	signal (SIGPIPE, SIG_IGN);


	int fd = tcp_listen (host, port);

	if (set_fl (fd, O_NONBLOCK) == -1)
	{
		return -1;
	}

	return fd;
}

//typedef void (*EVENT_CALLBACK) (int, int, void*);

using EVENT_CALLBACK = void (*)(int,int,void*);


EVENT_CALLBACK g_accept_ready;
EVENT_CALLBACK g_read_ready;

void reactor_init (int srv_socket, EVENT_CALLBACK accept_ready, EVENT_CALLBACK read_ready)
{
	::g_accept_ready = accept_ready;
	::g_read_ready = read_ready;

	int ready_count;
	int epoll_fd = epoll_create1 (EPOLL_CLOEXEC);
	u_int32_t msg_head;
	struct sockaddr_in peer_addr;
	socklen_t peerlen = sizeof peer_addr;
	int conn_fd;

	struct epoll_event event;
	event.data.fd = srv_socket;
	event.events = EPOLLIN;

	epoll_ctl (epoll_fd, EPOLL_CTL_ADD, srv_socket, &event);

	std::vector<struct epoll_event> vec_epoll_events (16);



	while (1)
	{
		ready_count = epoll_wait (epoll_fd, &vec_epoll_events[0], static_cast<int> (vec_epoll_events.size()), -1);

		if (ready_count == -1)
		{
			exit (EXIT_FAILURE);
		}

		if (static_cast<size_t>(ready_count) == vec_epoll_events.size ())
		{
			vec_epoll_events.resize(vec_epoll_events.size() * 2);
		}


		for (int i = 0; i < ready_count; i++)
		{
			if (vec_epoll_events[i].data.fd == srv_socket)
			{
				conn_fd = accept (srv_socket, (struct sockaddr*)&peer_addr, &peerlen);
				if (conn_fd == -1)
				{
					exit (EXIT_FAILURE);
				}

				fcntl (conn_fd, F_SETFL, fcntl (conn_fd, F_GETFL, 0) | O_NONBLOCK);

				event.data.fd = conn_fd;
				event.events = EPOLLIN;

				epoll_ctl (epoll_fd, EPOLL_CTL_ADD, conn_fd, &event);

				//On Accept

			}
			else
			{
				int len = recv (vec_epoll_events[i].data.fd, &msg_head, sizeof msg_head, MSG_PEEK);

				if (len == 0)
				{
					event = vec_epoll_events[i];
					epoll_ctl (epoll_fd, EPOLL_CTL_DEL, vec_epoll_events[i].data.fd, &event);

					// On Disconnect
				}

				if (len != sizeof msg_head) /*length is less than head*/
				{
					continue;
				}

				msg_head = ntohl (msg_head);

				{
					auto up_buf = std::make_unique<char[]> (msg_head);

					if (recv (vec_epoll_events[i].data.fd, up_buf.get(), msg_head + sizeof msg_head, MSG_PEEK) != msg_head + sizeof msg_head)
					{
						continue;
					}
				}

				// On Read
			}
		}

	}


}

