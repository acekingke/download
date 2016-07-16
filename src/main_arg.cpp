#include "main_arg.h"

main_arg::main_arg( int argc, char** argv )
{
	for (int i = 1; i < argc; i++)
	{
		args.push_back(argv[i]);
	}
}


