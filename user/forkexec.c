#include "kernel/types.h"
#include "user/user.h"
int main(){
    int pid;
    int status;
    pid = fork();
    if(pid == 0){
	char *argv[] = {"echo", "this", "is", "echo", 0};
	exec("echo", argv);
	printf("exec failed!\n");
	exit(1);
    }
    else {
	printf("parent waiting\n");
	wait(&status);
	printf("the child exit with status %d\n", status);
    }
    exit(0);
}
