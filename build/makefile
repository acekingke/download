C++=g++
PRJ_NAME=
srcpath=./
includepath = ./include

VPATH=$(srcpath)/src
CFLAGS= -fpic  -c -I $(includepath) -m64


%.o:%.c
	$(CC)  $(CFLAGS) $< -o $@
%.o:%.cpp 
	$(C++)  $(CFLAGS) $< -o $@

object_files = http.o main_arg.o md5.o test.o thread_template.o main.o mem_alloc.o


final_objects = download
all : $(final_objects)
	$(C++) $(CFLAGS) -o $@ $(object_files) -lpthread -lcurl
clean :
	 @rm -rf $(object_files)
	 @rm -rf $(final_objects)

rebuild	: clean	all
