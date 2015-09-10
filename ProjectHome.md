epoll + 线程池
做到支持 1:10000 以上的并发不成问题,呵呵

由于使用了 epoll 和 futex (添加中，.....) 和 clone 建立的共享全部内存的进程 （ 目前暂时使用线程）
基本上就没有打算支持 Linux 以外的平台， 呵呵.

因为其实是基于进程的，所以稳定性非常优秀。
又因为是共享了全部内存的进程，所以通信非常方便,高效

TODO:
> move locks to futex, we need faster speed
> use clone and create chared process instead of threads
> monitor SIGCHILD and clean up memory when process terminates un-expected. And RESTART dead processes.