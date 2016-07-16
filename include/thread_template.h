#ifndef THREAD_TEMPLATE_H
#define THREAD_TEMPLATE_H
#ifdef WIN32
#include <Windows.h>
typedef unsigned pthread_t;
#else
#include <pthread.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>
#endif
#include <deque>
#include <iostream>
#include <vector>
#include "mem_alloc.h"

class conditon;

class mutex{
public:
	mutex();
	~mutex();
	void lock();
	void unlock();

private:
#ifdef WIN32 
	CRITICAL_SECTION    m_lock;		/** 临界区对象*/
#else
	pthread_mutex_t     m_lock;
#endif
	friend class conditon;
	volatile bool is_locked;
};

class auto_mutex{
public:
	explicit auto_mutex(mutex *mu):m_mu(mu){m_mu->lock();}
	~auto_mutex(){m_mu->unlock();}
private:
	mutex* m_mu;
};
class conditon{
public:
	explicit conditon(mutex* mu);
	~conditon();
	void wait();
	void signal();
	void signal_all();
private:
	mutex* m_mu;

	mutex m_wait_mtx;
	long m_waiting;
#ifdef WIN32
	void * sem1_;
	void * sem2_;
#else
	pthread_cond_t m_cond_var;
#endif

};


class thread_template{

public:
	thread_template();

	virtual void run();
private:
	
};
class thread_pool{
public:
	thread_pool();
	thread_pool(long siz);
	~thread_pool();
	void start(); 
	void add_func(thread_template *func);
	void execute();
	void stop();
private:
	long						 m_n_of_thread;
	mutex						 m_thr_mutex;
	conditon				     m_convar;
	std::deque<thread_template*> m_thr_funcs;
	std::vector<pthread_t>		 m_threads;
	bool						 m_need_quit;
};
long pthread_self_id();
#endif