#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

struct Result
{
	int start;
	int end;
	int cnt;
};

/* Return 1 on prime number, 0 otherwise */
int prime(int n)
{
	int i, j = 0;

	for(i = 2; i * i <= n; i++)
		if(n % i == 0)
			return(0);
	return(1);
}

int main(int argc, char *argv[])
{
	struct Result result;			// Struct for result data
	int start, end, i, ret, cnt = 0;
	int fd;					// File descriptor
	int pid = getpid();			// Process ID

	/* Hack: get start and end from program args */
	start = atoi(argv[1]);
	end = atoi(argv[2]);

	/* Count prime numbers */
	for(i = start; i <= end; i++)
		if(prime(i))
			cnt = cnt + 1;

	/* Fill the struct with results */
	result.start = start;
	result.end = end;
	result.cnt = cnt;

	/* Open file "file" in write only mode, append data to file */
	fd = open("file", O_WRONLY | O_APPEND);
	if (fd < 0) {
		perror("opening error!");
		return -1;
	}

	printf("%d: locking file\n", pid);
	ret = lockf(fd, F_LOCK, 0);
	if (ret < 0) {
		perror("lock file");
		return -1;
	}
	printf("%d: file locked\n", pid);

	/* write results to file */
	ret = write(fd, &result, sizeof(result));
	if (ret < 0) {
		perror("write to file");
		return -1;
	}
	printf("%d: write status: %d\n", pid, ret);

	printf("%d: unlocking file\n", pid);
	ret = lockf(fd, F_ULOCK, 0);
	if (ret < 0) {
		perror("unlock file");
		return -1;
	}
	printf("%d: unlocked file\n", pid);

	/* close file */
	close(fd);

	printf("%d: in range %d-%d, found: %d prime numbers\n", pid, start, end, cnt);

	return 0;
}
