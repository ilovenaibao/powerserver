/*
 * typdefs.h
 *
 *  Created on: 2009-7-15
 *      Author: root
 */

#ifndef TYPDEFS_H_
#define TYPDEFS_H_

#include "stdafx.h"

class netserver;
class worker;

struct fd_server{
	int	fd;
	class netserver*server;
	class worker* 	Tworker;
};

struct EPOLL_THREAD_DATA{
	class netserver*server;
	void		*	common_buffer;
	size_t			common_buffer_size;
	// lock for protect the struct
	pthread_mutex_t	lock;
	int				epollfd;
	int	volatile	size_pollingfd;
};

struct SLAVE_TASKS{
	class worker* owner;
	class netserver *server;
	void * (* hard_task_func_ptr)(void *);
	void *   user_param;
};

struct SLAVE_THREAD_DATA{
	//loke for this
	pthread_mutex_t lock;
	//condition
	pthread_cond_t	cond;

	int volatile	must_Exit;
	std::list<pthread_t> pthreads;
	std::deque<SLAVE_TASKS>  tasks;
	inline SLAVE_THREAD_DATA()
	:pthreads()
	{
		pthread_mutex_init(&lock,0);
		pthread_cond_init(&cond,0);
		must_Exit = 0;
	}
//Do some clean up, haha
	inline ~SLAVE_THREAD_DATA()
	{
		must_Exit = 1;
		pthread_cond_broadcast(&cond);
		std::list<pthread_t>::iterator it;
		for (it = pthreads.begin(); it != pthreads.end(); ++it)
		{
			pthread_join(*it, 0);
		}
	}
};

static void * EPOLL_THREAD(struct EPOLL_THREAD_DATA * eptd);
static void * worker_slave(SLAVE_THREAD_DATA*);

#endif /* TYPDEFS_H_ */
