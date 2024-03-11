#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define READ 0
#define WRITE 1

void sieveProc(int *left){
    close(left[WRITE]);

    int prime, temp, pid, right[2];
    if(read(left[WRITE], &prime, sizeof(int)) == 0 ){
    	close(left[READ]);
	exit(0);
    }
    printf("prime %d\n", prime);
    pipe(right);
    if((pid = fork()) < 0){
    	printf("primes : fork error.");
	close(right[READ]);
	close(right[WRITE]);
	close(left[READ]);
	exit(1);
    }
    else if (pid > 0){
	close(right[READ]);
	while( read(left[READ], &temp, sizeof(int)) ){
	    if(temp % prime == 0) continue;
	    write(right[WRITE], &temp, sizeof(int));
	}
	close(right[WRITE]);
	wait(0);
	exit(0);
    }
    else{
	sieveProc(right);
	exit(0);
    }
}

void main(int argc, char **argv){ 
    int pid, fd[2];
    pipe(fd);
    if((pid = fork()) < 0){
  	printf("primes: fork failed.");
	exit(1);
    }
    else if (pid > 0){
    	close(fd[READ]);
	for(int i = 2; i <= 35; i++){
	    write(fd[WRITE], &i, sizeof(int));
	}
	close(fd[WRITE]);
	wait(0);//use wait to recollect resource.
	exit(0);
    }
    else {
	sieveProc(fd);
	exit(0);
    }
}
