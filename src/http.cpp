#include "http.h"


curl_singleton& curl_singleton::get_intance()
{
	static curl_singleton instance;
	return instance;
}

http_downloader::http_downloader():m_seg_num(1),m_thr_pool(1)
{
	curl_singleton::get_intance();
	m_curl = curl_easy_init();
	m_thr_pool.start();
}

http_downloader::http_downloader(long seg,  long pool_size ):
	m_seg_num(seg),m_thr_pool(pool_size)
{
	curl_singleton::get_intance();
	m_curl = curl_easy_init();
	m_thr_pool.start();
}

http_downloader::~http_downloader()
{
	curl_easy_cleanup(m_curl);
}

long http_downloader::get_size()
{
	double siz = 0;
	curl_easy_setopt(m_curl, CURLOPT_URL, m_url_str.c_str());
	curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
	curl_easy_perform(m_curl);
	curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &siz);
	return (long)siz;
}

void http_downloader::set_url(const char *url )
{
	m_url_str = std::string(url);
}

std::string http_downloader::parse_file_in_url()
{
	const char *p = strrchr(m_url_str.c_str(), '/');
	return std::string(p + 1);
}

void http_downloader::split()
{
	//split to serval segments, and build the split agss
	long file_size = get_size();
	if (file_size == -1)
	{
		std::cerr<<"cannot get the remote size ,check the url is corrent or your network can work"
			<<std::endl;
		return;
	}
	long seg_size, rest;
	long split_seg = m_seg_num;
	void* buf = NULL;
	split_arg_t *arg;
	long start = 0;
	char tmp[100] = {0,};
	std::string file_str = parse_file_in_url();
	range_down_thread *rang_thr;

	if (file_size > 1024){
		seg_size = file_size / m_seg_num;
		
	}else{
		seg_size = file_size;
		split_seg = 1;
	}

	rest = file_size;
	for (int i = 0; i < split_seg; i++)
	{
		buf = m_allocter.allocate_aligned(sizeof(split_arg_t));
		arg = new(buf) split_arg_t;
		arg->start = start;
		arg->end = start + seg_size;		
		rest -= seg_size;
		//last one
		if (i == split_seg - 1 && rest > 0)
		{
			arg->end += rest;
		}
		sprintf(tmp, "_%d", i);
		arg->file_name = file_str;
		arg->file_name.append(tmp).append(".tmp");
		start = arg->end + 1;
		m_splits.push_back(arg);
		rang_thr = new range_down_thread(m_url_str.c_str(), 
			arg->file_name.c_str(), arg->start, arg->end);
		m_thr_pool.add_func(rang_thr);
	}
	//wait them complete
	m_thr_pool.stop();
	std::vector<split_arg_t*>::iterator it;
	std::ifstream instream;
	std::ofstream outstream(file_str.c_str(), std::ios::binary | std::ios::out);
	outstream.close();
	mem_allocter tmp_allocter;
	buf = tmp_allocter.allocate_aligned(2 * seg_size);

	md5_state_s md5_state;
	byte		digest[16];
	md5_init(&md5_state);
	for(it = m_splits.begin(); it != m_splits.end(); it++)
	{
		arg = *it;
		instream.open(arg->file_name.c_str(), std::ios::binary | std::ios::in);
		instream.seekg(0, instream.end);
		file_size = instream.tellg();
		instream.seekg(0, instream.beg);
		instream.read((char*)buf, file_size);
		md5_append(&md5_state, (byte*)buf, file_size);
		instream.close();
		outstream.open(file_str.c_str(), std::ios::binary | std::ios::out | std::ios::app);
		outstream.write((char*)buf, file_size);
		outstream.close();
	}
	md5_finish(&md5_state, digest);
	std::cout<<"NOW THE MD5 DIGEST IS:"<<std::endl;
	for(int i = 0; i < 16; i++)
	{
		printf("%02X", digest[i]);
	}
	std::cout<<std::endl;

}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	fflush(stream);
	printf("Thread %ld down load %d\n", pthread_self_id(), written);
	return written;
}

range_down_thread::range_down_thread( const char *url_str, 
	const char *file_name, int start, int end ):
	m_file_name(file_name),m_fp(NULL)
{
	curl_singleton::get_intance();
	m_curl = curl_easy_init();

	m_fp = fopen(file_name, "wb");
	curl_easy_setopt(m_curl, CURLOPT_URL, url_str);
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS , 5L);
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 300L);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, m_fp);
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &write_data);
	char tmp[1024] = {0,};
	
	sprintf(tmp, "%d-%d", start, end);
	curl_easy_setopt(m_curl, CURLOPT_RANGE, tmp);

}

range_down_thread::~range_down_thread()
{
	if (m_fp)
		fclose(m_fp);
	curl_easy_cleanup(m_curl);
}

void range_down_thread::run()
{
	curl_easy_perform(m_curl);
}
