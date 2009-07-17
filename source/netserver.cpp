/*
 * netserver.cpp
 *
 *  Created on: 2009-7-14
 *      Author: microcai
 */

#include "stdafx.h"

#include "netserver_exception.h"

#include "./typedefs.h"
#include "worker.h"
#include "netserver.h"

netserver::netserver(int listensocket)
:m_listensock(listensocket),
slave_thread_data(0)
{
	//slave_thread_data = 0;//pthread_mutex_init(&_slave_lock,0);
}

netserver::~netserver()
{
	close(m_listensock);

	for(int i=0;i<epoll_threads_munber;++i)
	{
		pthread_join(epoll_threads[i],0);
	}

	delete ((EPOLL_THREAD_DATA*) epted);
	free(epoll_threads);
	if (slave_thread_data)
		delete (SLAVE_THREAD_DATA*) slave_thread_data;
}

int netserver::start_epoll_threads(int number)
{
	epoll_threads_munber = number;
	epted = (void*)(new EPOLL_THREAD_DATA);
	this->epoll_threads = (pthread_t *) calloc(epoll_threads_munber, sizeof(pthread_t));
	((EPOLL_THREAD_DATA*) epted)->server = this;
	((EPOLL_THREAD_DATA*) epted)->epollfd = epoll_create(100000);
	pthread_mutex_init(&(((EPOLL_THREAD_DATA*) epted)->lock), 0);
	((EPOLL_THREAD_DATA*) epted)->size_pollingfd=0;

	for(int i=0;i<number;++i)
	{
		pthread_create(&epoll_threads[i], 0, (void *(*)(void*)) EPOLL_THREAD,epted);
	}
	return 0;
}

int inline netserver::insert_to_epoll(int fd, worker* __worker,epoll_event *epev)
{
	EPOLL_THREAD_DATA * toinsert;

	toinsert = ((EPOLL_THREAD_DATA*) epted);
	//ok, insert one;
	pthread_mutex_lock(&toinsert->lock);
	toinsert->size_pollingfd++;
	pthread_mutex_unlock(&toinsert->lock);
	//real insert
	__worker->epoller = toinsert;
	return epoll_ctl(toinsert->epollfd, EPOLL_CTL_ADD, fd, epev);
}

int netserver::erase_from_epoll(worker* _worker)
{
	pthread_mutex_lock(&_worker->epoller->lock);
	_worker->epoller->size_pollingfd--;
	pthread_mutex_unlock(&_worker->epoller->lock);
	int ret = epoll_ctl(_worker->epoller->epollfd, EPOLL_CTL_DEL,_worker->m_fd, 0);
	close(_worker->m_fd);
	this->deleteWorker(_worker);
	return ret;
}

static void * start_accept_async(void* _this)
{
	pthread_detach(pthread_self());
	((netserver*)_this)->start_accept();
	return 0;
}

int netserver::start_accept_async()
{
	pthread_t pid;
	return pthread_create(&pid,0,::start_accept_async,this);
}

int netserver::start_accept()
{
	sockaddr saddr;
	socklen_t saddr_len;
	epoll_event epev;

	int newfd;
	class worker * _worker;
	_worker = newWorker();
	_worker->server = this;
	while (newfd = accept(m_listensock, &saddr, &saddr_len))
	{
		if(newfd < 0)
			break;
		_worker->m_client_sock = saddr;
		_worker->m_client_sock_len = saddr_len;
		_worker->m_fd = newfd;
		epev.events=0;
		/*
		 * Check whether we need to send messages before the client
		 * send any data to us
		 */
		switch(_worker->check_client())
		{
			// let the epoll threads monitoring it;
		case 1:
			epev.events = EPOLLOUT;
		case 0:
			epev.data.ptr = _worker;
			epev.events |= EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLPRI;
			insert_to_epoll(newfd,_worker,&epev);
			break;
		case -1:
			//disconnect the client
			close(newfd);
			continue;
		}
		_worker = newWorker();
		_worker	->server = this;
	}
	// now , do cleanup

}

int netserver::get_running_slave_threads()
{
	if (slave_thread_data)
		return ((SLAVE_THREAD_DATA*)slave_thread_data)->pthreads.size();
	else return 0;
}

int netserver::stop_one_slave_thread()
{
	SLAVE_TASKS	task;
	task.hard_task_func_ptr = 0;
	task.owner = 0;
	task.server = this;
	task.user_param = 0;

	SLAVE_THREAD_DATA * p = (SLAVE_THREAD_DATA*) slave_thread_data;

	if(get_running_slave_threads() > 0)
	{
		pthread_mutex_lock(&p->lock);
		// check flags
		p->tasks.push_back(task);
		pthread_mutex_unlock(&p->lock);
	}
	// What ?! Call this befor start any slave threads ?!
	else	return -1;
}

int netserver::start_one_slave_threads()
{
	if (!slave_thread_data)
	{
		slave_thread_data = (void*) (new SLAVE_THREAD_DATA);
	}
	pthread_t pid;
	int ret = pthread_create(&pid, 0, (void *(*)(void*)) worker_slave, slave_thread_data);
	((SLAVE_THREAD_DATA*)slave_thread_data)->pthreads.push_back(pid);
	return ret;
}

static void * EPOLL_THREAD(struct EPOLL_THREAD_DATA * eptd)
{
	int size = eptd->size_pollingfd +1 ;
	int ret ;
	epoll_event epevent;

	struct epoll_event * epevent_ptr;
	epevent_ptr = (typeof(epevent_ptr)) calloc(size,sizeof(struct epoll_event));
	eptd->common_buffer_size = getpagesize() * 2;
	eptd->common_buffer = mmap(0, eptd->common_buffer_size, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

	for(;;)
	{
		if(size < eptd->size_pollingfd || size > eptd->size_pollingfd*2 )
		{
			if(eptd->size_pollingfd)
			{
				free(epevent_ptr);
				epevent_ptr = (typeof(epevent_ptr)) calloc(eptd->size_pollingfd
						+ 4, sizeof(struct epoll_event));
				size = eptd->size_pollingfd + 1;
			}
		}

		ret = epoll_wait(eptd->epollfd, epevent_ptr, size, -1);

		for (int i = 0; i < ret; ++i)
		{
			class worker *pworker = (class worker*)(epevent_ptr[i].data.ptr);

			// Do things here;
			//say commit worker to the working threads/processes.
			if (epevent_ptr[i].events & EPOLLERR)
			{
				pworker->OnClose(0x4);//00000000100B
				eptd->server->erase_from_epoll(pworker);
			}
			else if(epevent_ptr[i].events & EPOLLRDHUP)
			{
				 // 连接断开
				pworker->OnClose(0x0); //00000000000010B
				eptd->server->erase_from_epoll(pworker);
			}
			else if (epevent_ptr[i].events & EPOLLHUP)
			{
				if (pworker->OnClose(0x2) != 0)
					eptd->server->erase_from_epoll(pworker);
			}
			else
			{
				epevent.data.ptr = epevent_ptr[i].data.ptr;
				epevent.events = EPOLLHUP | EPOLLERR | EPOLLPRI;

				if (epevent_ptr[i].events & EPOLLPRI)
				{
					ret = pworker->OnPriData();
					if(ret ==-1)
					{
						epevent_ptr[i].events =0;
					}
				}
				if (epevent_ptr[i].events & EPOLLOUT)
				{
					ret = pworker->OnWrite();
					switch(ret)
					{
					case 0:
						break;
					case -1:
						epevent_ptr[i].events = 0;
						break;
					default:
						epevent.events |= ret;
					}
				}
				if (epevent_ptr[i].events & EPOLLIN)
				{
					ret = pworker->OnRead();
					switch (ret)
					{
					case 0:
						break;
					case -1:
						epevent_ptr[i].events = 0;
						break;
					default:
						epevent.events |= ret;
					}
				}
				switch (ret)
				{
				case 0:
					break;
				case -1:
					pworker->OnClose(0);
					eptd->server->erase_from_epoll(pworker);
					break;
				default:
					epoll_ctl(eptd->epollfd, EPOLL_CTL_MOD, pworker->m_fd,&epevent);
				}
			}
		}
		if (ret == -1)
		{
			if (errno != EINTR)
			{
				break;
			}
		}
	}
	munmap(eptd->common_buffer,eptd->common_buffer_size);
	free(epevent_ptr);
}

static void * worker_slave(SLAVE_THREAD_DATA*std_ptr)
{
	SLAVE_TASKS task;
	while (!std_ptr->must_Exit)
	{
		//检查队列
		pthread_mutex_lock(&std_ptr->lock);

		if (std_ptr->tasks.size() == 0)
		{
			while (std_ptr->tasks.size() == 0 && !std_ptr->must_Exit)
				pthread_cond_wait(&std_ptr->cond, &std_ptr->lock);
			if (std_ptr->must_Exit)
			{
				pthread_mutex_unlock(&std_ptr->lock);
				break;
			}
		}
		task = *std_ptr->tasks.begin();
		std_ptr->tasks.pop_front();
		pthread_mutex_unlock(&std_ptr->lock);
		if(task.hard_task_func_ptr)
			task.hard_task_func_ptr(task.user_param);
		else if(task.owner == NULL)
		{
			pthread_detach(pthread_self());
			break;
		}
		//服务
	}
	return 0;
}

