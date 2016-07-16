#include "test.h"
#include "main_arg.h"
int main(int argc , char** argv)
{
	//test002();

	long thr = 1, seg_num = 1;
	if (argc == 1)
	{
		std::cout<<"download [-s -t] url"<<std::endl;
		std::cout<<"-s segment number, -t thread number"<<std::endl;
	}
	main_arg args (argc, argv);
	const char *opt = args.arg_opt("-s");
	if (opt)
	{
		seg_num = atol(opt);
	}
	opt = args.arg_opt("-t");
	if (opt)
	{
		thr = atol(opt);
	}
	http_downloader http_down(seg_num, thr);
	http_down.set_url(args.arg_last());
	http_down.split();

}