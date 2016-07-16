#include "thread_template.h"
#include <iostream>
#ifdef WIN32
#include <PROCESS.H>
#else
//for linux
#endif

int pthread_join(pthread_t thread, void **value_ptr)
{
	DWORD  ret;
	HANDLE handle;

	handle= OpenThread(SYNCHRONIZE, FALSE, thread);
	if (!handle)
	{
		errno= EINVAL;
		goto error_return;
	}

	ret= WaitForSingleObject(handle, INFINITE);

	if(ret != WAIT_OBJECT_0)
	{
		errno= EINVAL;
		goto error_return;
	}

	CloseHandle(handle);
	return 0;

error_return:
	if(handle)
		CloseHandle(handle);
	return -1;
}

long pthread_self_id()
{
#ifdef WIN32
	return GetCurrentThreadId();
#else
	return (long)pthread_self();
#endif 
}

#ifdef WIN32
#endif // WIN32
thread_template::thread_template()
{
	
}

void thread_template::run()
{
	std::cout<<"thread_template run !"<<std::endl;
}



mutex::mutex()
{
#ifdef WIN32
	::InitializeCriticalSection(&m_lock);
#else
	pthread_mutex_init(&m_lock, NULL);
#endif
	is_locked = false;

}

mutex::~mutex()
{
#ifdef WIN32
	::DeleteCriticalSection(&m_lock);
#else
	pthread_mutex_destroy(&m_lock);
#endif  
	is_locked = false;
}

void mutex::lock()
{
#ifdef WIN32
	::EnterCriticalSection(&m_lock);
#else
	pthread_mutex_lock(&m_lock);
#endif
	is_locked = true;
}

void mutex::unlock()
{
	is_locked = false;
#ifdef WIN32
	 ::LeaveCriticalSection(&m_lock);
#else
	pthread_mutex_unlock(&m_lock);
#endif
}

conditon::conditon( mutex* mu ):
	m_waiting(0), 
	m_mu(mu) 	
#ifdef WIN32
	,sem1_(::CreateSemaphore(NULL, 0, 10000, NULL)), 
	sem2_(::CreateSemaphore(NULL, 0, 10000, NULL)) 
#endif
{
#ifndef WIN32
	pthread_cond_init(&m_cond_var, NULL);
#endif
}

void conditon::wait()
{
#ifdef WIN32
	m_wait_mtx.lock();
	++m_waiting;
	m_wait_mtx.unlock();

	m_mu->unlock();
	
	::WaitForSingleObject(sem1_, INFINITE);
	::ReleaseSemaphore(sem2_, 1, NULL);
	m_mu->lock();
#else
	 pthread_cond_wait(&m_cond_var, &m_mu->m_lock);
#endif
}

void conditon::signal()
{
#ifdef WIN32
	m_wait_mtx.lock();
	if (m_waiting > 0) {
		--m_waiting;
		::ReleaseSemaphore(sem1_, 1, NULL);
		::WaitForSingleObject(sem2_, INFINITE);
	}
	m_wait_mtx.unlock();
#else
	pthread_cond_signal(&m_cond_var);
#endif // WIN32
}

void conditon::signal_all()
{
#ifdef WIN32
	while (m_waiting != 0)
	{
		signal();
	}
#else
	pthread_cond_broadcast(&m_cond_var);
#endif // WIN32
}

conditon::~conditon()
{

#ifdef WIN32
	::CloseHandle(sem1_);
	::CloseHandle(sem2_);
#else
	pthread_cond_destroy(&m_cond_var);
#endif // WIN32

}



unsigned int 
#ifdef WIN32
__stdcall
#endif
thr_fun(void *arg)
{
	thread_pool *thr_arg = (thread_pool*)arg;
	thr_arg->execute();
	return 0;
}


void thread_pool::start()
{
	pthread_t thid;
	for (int i = 0; i < m_n_of_thread; i++) {
#ifdef WIN32
		::_beginthreadex(NULL, 0, thr_fun, this, 0, &thid);
#else
		pthread_create(&thid, NULL, thr_fun, (void*) this);
#endif
		m_threads.push_back(thid);
	}
}




thread_pool::thread_pool():
	m_n_of_thread(1),
	m_convar(&m_thr_mutex),
	m_need_quit(false)
{

}

thread_pool::thread_pool( long siz ):
	m_n_of_thread(siz),
	m_convar(&m_thr_mutex),
	m_need_quit(false)
{

}

thread_pool::~thread_pool()
{
	if (m_need_quit)
		return;
	else
		stop();
}


void thread_pool::execute()
{
	thread_template *thr = NULL;
	std::cout<<"Now execute "<<std::endl;
	
	while(true)
	{
		m_thr_mutex.lock();
		//if empty unlock and waiting
		while ((!m_need_quit) && (m_thr_funcs.empty())) {
	
			m_convar.wait();
		
		}
		if (m_need_quit && (m_thr_funcs.empty())) {
			m_thr_mutex.unlock();
			return;
		}
		//fetch and run
		thr = m_thr_funcs.front();

		m_thr_funcs.pop_front();
		
		m_thr_mutex.unlock();

		thr->run();
		delete thr;
		
	}
}

void thread_pool::add_func( thread_template *func )
{
	m_thr_mutex.lock();

	
	m_thr_funcs.push_back(func);

	m_convar.signal(); 

	m_thr_mutex.unlock();
}

void thread_pool::stop()
{
	m_thr_mutex.lock();
	m_need_quit = true;
	m_thr_mutex.unlock();
	m_convar.signal_all();

	for (int i = 0; i < m_n_of_thread; i++)
	{
		void* result;
		pthread_join(m_threads[i], &result);

		m_convar.signal_all();
	}
	
}
