/*
 * worker.h
 *
 *  Created on: 2009-7-15
 *      Author: root
 */

#ifndef WORKER_H_
#define WORKER_H_
#include <sys/socket.h>
//全部虚函数，这里留给 用户 自己实现对连接的处理
class netserver;
struct EPOLL_THREAD_DATA;

class worker
{
public:
	worker(){}
	virtual ~worker(){}
public:
	/* please use commonbuffer , the commonbuffer is pre-allocated by the epolling
	 * thread , and will only be freed when the thread terminates. If you have to
	 * share the recv buffer with other threads or must use somewhere else, you can
	 * not use the pre-allocated buffer, otherwise , I strongly recommend that you
	 * use this. */
	char * GetCommonBuffer();
	size_t GetCommonBufferSize();

	/*
	 * If you need to do things that might cost a lot of time and blocked for a while
	 * We strongly recommend that you commit the work to the slaver. The slave is some
	 * pre-forked thread/LWP , you can commit to existing slave, and start new slave
	 * at your wish.
	 * */
	int	   CommitWork(int flags,void * (* hard_task)(void *), void * param);
protected:
	/*
	 * int reason
	 *
	 * first bit  if set means close by server side , not set means by client side
	 * 2nd bit if set means half close , not set means full close
	 * 3rd bit if set means closed by internal err.
	 *
	 * retval : 0 : do not change epoll state
	 * 			-1: want to close socket.
	 * retval is ingored if reason is internal err.
	 */
	virtual int OnClose(int reason);

	/*
	 * called by accepting thread, so must be very very quick
	 * retval : 0 : wait for read (NORMAL/DEFAULT if not override)
	 * 		  : 1 : need to send data, please monitor EPOLLOUT
	 * 		  : -1: please close the socket, I do not want to serve this client
	 * if it's always 0, please don't override
	 */
	virtual int check_client();

protected:

	/*
	 * called if set EPOLLIN and there is data send to you
	 * called by epolling thread, must be quick.
	 * retval can be combined woth | except -1
	 * retval : 0 : do not change epoll state
	 * 			-1: want to close socket.
	 * other wise, return the EPOLL combinations
	 */
	virtual int OnRead();

	/*
	 * called if you set EPOLLOUT and when you can write
	 * called by epolling thread, must as quick as possible
	 * retval can be combined woth | except -1
	 * retval : 0 : do not change epoll state, want to send more data
	 * 			-1: want to close socket.
	 * other wise, return the EPOLL combinations
	 */
	virtual int OnWrite();

	/* called when there is urgen data to be read.
	 * called by epolling thread, must be quick.
	 *
	 * retval : 0 : do not change epoll state
	 * 			-1: want to close socket.
	 * other wise, return the EPOLL combinations
	 * If you just don't want to recv OOB data, please don't override the function
	 */
	virtual int OnPriData();

protected:
	sockaddr		m_client_sock;
	socklen_t		m_client_sock_len;

	//WARINING	: don't close the socket, we'll close it
	int				m_fd;
	class netserver*server;

protected:/********************************************************/
	// internal use, no meanns for users
	EPOLL_THREAD_DATA*epoller;
	friend class netserver;
	friend void * EPOLL_THREAD(struct EPOLL_THREAD_DATA * eptd);
	/**************************************************************/
};

#endif /* WORKER_H_ */
