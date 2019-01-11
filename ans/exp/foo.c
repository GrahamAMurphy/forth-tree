#include <stdio.h>
#include <dlfcn.h>

main()
{
	void *handle;
	if((handle=dlopen("/usr/lib/libc.so.0.15",1)) == NULL){
		fprintf(stderr,"foo: %s\n", dlerror());
		exit(1);
	}
	printf("strcpy = %x\n", dlsym(handle,"strcpy"));
	printf("strcat = %x\n", dlsym(handle,"strcat"));
	printf("printf = %x\n", dlsym(handle,"printf"));
	((void (*)())(dlsym(handle,"printf")))("hello world!\n");
	return(0);
}
