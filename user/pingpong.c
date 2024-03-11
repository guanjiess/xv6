#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

int main(int argc, char **argv){
    int p2c[2], c2p[2];
    int pid;
    char p2c_buf[64], c2p_buf[64];

    pipe(p2c);
    pipe(c2p);

    if((pid = fork()) < 0){
	printf("fork error...\n");
	exit(1);
    } else if (pid == 0){
	close(p2c[WRITE]);
	close(c2p[READ]);

	read(p2c[READ], p2c_buf, sizeof(p2c_buf));
	close(p2c[READ]);

	printf("%d: received %s\n", getpid(), p2c_buf);
	write(c2p[WRITE], "pong", 4);
	close(c2p[WRITE]);
	exit(0);
    } else {
	close(c2p[WRITE]);
	close(p2c[READ]);

	write(p2c[WRITE], "ping", 4);
	close(p2c[WRITE]);

	read(c2p[READ], c2p_buf, sizeof(c2p_buf));
	close(c2p[READ]);
    	
	printf("%d: received %s\n", getpid(), c2p_buf);
	exit(0);
    }

}
