/*
 * test_main.cpp
 *
 *  Created on: 2009-7-14
 *      Author: root
 */

#include "stdafx.h"

#include "netserver.h"
#include "worker.h"

class myworker:public worker
{
protected:
	int check_client(){return 0;}
	int OnRead()
	{
//		worker::OnRead();
		int ret = read(m_fd,GetCommonBuffer(),GetCommonBufferSize());

		write(2, this->GetCommonBuffer(),ret);
		return 0;
	}

};


class mynetserver:public netserver
{
	int fd;
public:
	mynetserver(int port):
	netserver(socket(AF_INET,SOCK_STREAM,0))
	{
		sockaddr_in ad={0};
		ad.sin_family = AF_INET;
		ad.sin_addr.s_addr = 0;
		ad.sin_port = htons(port);

		if( this->m_listensock <0)
		{
			throw (errno);
		}
		int opt = 1;
		setsockopt(m_listensock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		if (bind(m_listensock, (sockaddr*)&ad, INET_ADDRSTRLEN) < 0)
		{
				throw(errno);
		}

		if (listen(m_listensock, BACKLIST) < 0)
		{
			throw(errno);
		}

	}
protected:
	class worker * newWorker()
	{
		return new myworker;
	}
	void deleteWorker(class worker* w)
	{
		delete w;
	}
};

int main()
{
	mynetserver p(1280);
	p.start_epoll_threads(1);
	p.start_accept();

	return 0;
}
