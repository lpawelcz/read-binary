#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

struct Result
{
	int start;
	int end;
	int cnt;
};

int main(int argc, char *argv[])
{
	struct Result result;
	int status, i, j, fd, start, end, ret, sum = 0;
	int nproc;				/* Process count */
	int rpp;				/* Range per process */
	int pid = getpid();
	time_t t1, t2;
	char start_arg[5], end_arg[5];

	/* Create or clear file if it exists */
	fd = open("file", O_CREAT | O_TRUNC | O_RDWR, 0600);
	if (fd < 0) {
		perror("opening error!");
		return -1;
	}

	start = atoi(argv[1]);
	end = atoi(argv[2]);
	nproc = atoi(argv[3]);

	if (start > end) {
		printf("Start should be lower than end\n");
		exit(-1);
	}

	if (nproc < 1) {
		printf("nproc must be greater than zero\n");
		exit(-1);
	}

	if (argc != 4) {
		printf("bad args cnt\n");
		exit(-1);
	}

	rpp = (end - start) / nproc;		/* Calculate range per process */
	t1 = time(NULL);			/* Start time measurement */

	for (i = 1; i <= nproc; i++) {
		/* Spawn new process */
		if ((pid = fork()) == 0) {
			/* Prepare ranges as args for process */
			int s = 1 + (i - 1) * rpp;
			int e = (i - 1) * rpp + rpp;

			sprintf(start_arg, "%d", s);
			sprintf(end_arg, "%d", e);

			/* Execute prime numbers counting in newly spawned process */
			execl("./count", "count", start_arg, end_arg, NULL);
		}
	}

	/* This part of code executes only in parent proces */
	for (i = 0; i < nproc; i++) {
		/* wait for one of the processes to finish */
		pid = wait(&status);

		printf("parent: locking file\n");
		ret = lockf(fd, F_LOCK, 0);
		if (ret < 0) {
			perror("lock file");
			return -1;
		}
		printf("parent: file locked\n");

		/* read from file to result structure */
		ret = read(fd, &result, sizeof(result));

		if (ret < 0) {
			perror("read to file");
			return -1;
		}
		printf("parent: read status: %d\n", ret);

		printf("parent: unlocking file\n");
		ret = lockf(fd, F_ULOCK, 0);
		if (ret < 0) {
			perror("unlock file");
			return -1;
		}
		printf("parent: unlocked file\n");

		printf("Read from other process:\n");
		printf("start = %d end = %d cnt = %d\n",
			result.start, result.end, result.cnt);
		sum += result.cnt;		/* sum = sum + result.cnt; */
	}
	/* close file */
	close(fd);

	/* All partial results are received, stop time measurement */
	t2 = time(NULL);

	printf("\n\nPrime numbers in range %d-%d: %d\n", start, end, sum);
	printf("time: %ld\n", t2-t1);

	return 0;
}
