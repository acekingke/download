#include "test.h"

#include "thread_template.h"
#include <fstream>
#include "md5.h"
//test the mem_allocter
void test001()
{
	mem_allocter alloctor;
	char *p = alloctor.allocate_aligned(10);
	strcpy(p, "helloworld");
	std::cout<<p<<" endl"<<std::endl;
}

class thr_one_class : public thread_template
{
public:
	thr_one_class():m_size(0){}
	thr_one_class(int sz){m_size = sz;}
	virtual void run(){
		std::cout<<"This is thr_one_class "<<m_size<< " zzz"<<std::endl;
		std::string str("iut");
		char tmp[100];
		itoa(m_size, tmp, 10);
		str += std::string(tmp);
		std::ofstream my_file(str, std::ios::out | std::ios::binary);
		char buf[100];
		memset(buf, 0x11, sizeof(buf));
		my_file.write(buf, sizeof(buf));
		

	}
private:
	int m_size ;
};

void test002()
{
	thread_pool thr_pool(2);
	thr_pool.start();
	thr_one_class *one = new thr_one_class;
	thr_one_class *two = new thr_one_class(2);
	//thr_pool.add_func(one);
	//thr_pool.add_func(two);
	thr_pool.stop();
}

void test003()
{
	http_downloader http_down;
	http_down.set_url("http://www.execve.net/curl/curl-7.49.1.tar.gz");
	std::cout<<" NOw test 003 "
		<<http_down.get_size()<<std::endl;
	std::cout<<" NOw test 003 "
		<<http_down.get_size()<<std::endl;
	std::cout<<"download the file "<<http_down.parse_file_in_url().c_str()<<std::endl;
	md5_state_s md5_state;
	byte		digest[16];
	md5_init(&md5_state);
	md5_append(&md5_state, (const byte*)"Hello", strlen("Hello"));
	md5_finish(&md5_state, digest);
	for (int i = 0; i < sizeof(digest); i++)
	{
		printf("%02X", digest[i]);
	}
	printf("\n");
	return ;
}
void test004()
{
	http_downloader http_down(2, 2);
	http_down.set_url("http://www.execve.net/curl/curl-7.49.1.tar.gz");
	http_down.split();
}