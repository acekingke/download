#ifndef MAIN_ARG_H
#define MAIN_ARG_H
#include <vector>
class main_arg
{
public:
	explicit main_arg(int argc, char** argv);
	~main_arg(){};
	const char* arg_last(){return args.back().c_str();}
	const char*		 arg_opt(char* opt){
		std::vector<std::string>::iterator it;
		for (it = args.begin(); it!=args.end(); it++)
		{
			if ((*it).compare(opt) == 0){
				return std::next(it, 1)->c_str();
			}
		}
		return NULL;
	}
protected:
private:
	std::vector<std::string> args;
};



#endif