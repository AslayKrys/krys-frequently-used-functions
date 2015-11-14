#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/epoll.h>
#include "Krysock.h"
#include <vector>
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

typedef void (*EVENT_CALLBACK) (int, int, void*);

EVENT_CALLBACK g_accept_ready;
EVENT_CALLBACK g_read_ready;

void reactor_init (int srv_socket, EVENT_CALLBACK accept_ready, EVENT_CALLBACK read_ready)
{
	::g_accept_ready = accept_ready;
	::g_read_ready = read_ready;

	int ready_count;
	int epoll_fd = epoll_create1 (EPOLL_CLOEXEC);

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
				if (accept_ready )
				accept_ready (vec_epoll_events[i].data.fd, 0, nullptr);
			}
		}

	}


}

