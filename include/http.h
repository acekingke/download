#ifndef HTTP_H
#define HTTP_H
#include <fstream>
#include "curl/curl.h"
#include "thread_template.h"
#include "md5.h"
class curl_singleton{
public:
	static curl_singleton& get_intance();
	
	curl_singleton(){
		curl_global_init(CURL_GLOBAL_ALL);
	}
	~curl_singleton(){
		curl_global_cleanup();
	}
private:
	mutex m_mu;

};
class range_down_thread:public thread_template
{

public:
	explicit range_down_thread(const char *url_str, 
			const char *file_name,  int start, int end);
	~range_down_thread();
	virtual void run();
private:	
	std::string m_file_name;
	CURL* m_curl;
	FILE *m_fp;
};

typedef 
struct split_arg_struct{
	std::string file_name;
	long start, end;
}split_arg_t;

class http_downloader
{
public:
	http_downloader();
	http_downloader(long seg_n, long pool_size);
	void set_url(const char *url);
	long get_size();
	void split();
	std::string parse_file_in_url();
	~http_downloader();
protected:
private:
	long m_seg_num;
	CURL *m_curl;
	std::string m_url_str;
	std::vector<split_arg_t*> m_splits;
	mem_allocter m_allocter;
	thread_pool		m_thr_pool;
};
#endif