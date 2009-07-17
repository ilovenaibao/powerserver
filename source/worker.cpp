/*
 * worker.cpp
 *
 *  Created on: 2009-7-15
 *      Author: root
 */

#include "stdafx.h"

#include "./typedefs.h"
#include "worker.h"
#include "netserver.h"

size_t worker::GetCommonBufferSize()
{
	return epoller->common_buffer_size;
}

char * worker::GetCommonBuffer()
{
	return (char*)(epoller->common_buffer);
}

int worker::check_client()
{
	return fcntl(m_fd,F_SETFL,fcntl(m_fd,F_GETFL)|O_NONBLOCK);
}

int worker::OnRead()
{
	read(m_fd,GetCommonBuffer(),GetCommonBufferSize());
	return 0;
}

int worker::OnClose(int reason)
{
	return 0;
}

int worker::OnWrite()
{
	return 0;
}

int worker::OnPriData()
{
	recv(m_fd,GetCommonBuffer(),GetCommonBufferSize(),MSG_OOB);
	return 0;
}

int worker::CommitWork(int flags,void * (* hard_task)(void *), void * param)
{
	SLAVE_TASKS	task;
	task.hard_task_func_ptr = hard_task;
	task.owner = this;
	task.server = server;
	task.user_param = param;

	if (server->get_running_slave_threads() < 1)
	{
		server->start_one_slave_threads();
	}
	SLAVE_THREAD_DATA * slave_thread_data = (SLAVE_THREAD_DATA*)(server->getslave_thread_data());

	pthread_mutex_lock(&slave_thread_data->lock);

	// check flags

	slave_thread_data->tasks.push_back(task);


	pthread_mutex_unlock(&slave_thread_data->lock);
	pthread_cond_signal(&slave_thread_data->cond);
}
