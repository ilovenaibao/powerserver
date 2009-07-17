/*
 * netserver.h
 *
 *  Created on: 2009-7-14
 *      Author: root
 */

#ifndef NETSERVER_H_
#define NETSERVER_H_

#include <sys/epoll.h>
#include <netinet/in.h>
#include <pthread.h>

#define BACKLIST 30

struct EPOLL_THREAD_DATA;


class netserver
{
public:
	// 这个接口必须已经被 ”listen“
	netserver(int listensocket);
	virtual ~netserver();
public:
	//Call this befor start worker threads/process
	int start_epoll_threads(int number = 4);

	// Call this befor commit any tasks
	int start_one_slave_threads();
	int stop_one_slave_thread();
	int	get_running_slave_threads();

	//Will start accepting new connetctions, block forever.
	int	start_accept();
	//Will start accepting new connetctions async
	int	start_accept_async();

protected:
	// 请自己派生 worker 类 来处理客户连接 ， 并在此中返回new 出的 自己派生的类
	virtual class worker * newWorker()=0;
	virtual void deleteWorker(class worker*)=0;

protected:
	int		insert_to_epoll(int fd, worker * worker,epoll_event *);
	int		erase_from_epoll(worker *worker);
protected:
	int 		m_listensock;
	int 		epoll_threads_munber;
	void*		epted;
	pthread_t *	epoll_threads;
	void*		slave_thread_data;

	/*===============================================================*/
public: // warnning , do not use it, internal use only !
	inline void*	getslave_thread_data(){return slave_thread_data;}
	friend void* 	EPOLL_THREAD(struct EPOLL_THREAD_DATA *);
};

#endif /* NETSERVER_H_ */
