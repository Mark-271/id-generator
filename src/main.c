#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define THR_NUM 10

static pthread_t thr_ids[THR_NUM];
static bool finished;
static pthread_mutex_t lock;

/* Function to generate unique ids */
static int gen_id(void)
{
	static int id;
	int tmp;

	pthread_mutex_lock(&lock);
	tmp = id++; /* RMW, non-atomic */
	pthread_mutex_unlock(&lock);

	return tmp;
}

static void print_ids(void)
{
	while (!finished)
		printf("%d\n", gen_id());
}

/* Function to be operated by thread */
static void *thread_func(void *data)
{
	int err;
	unsigned long ret = 0;
	size_t i = (size_t)data;

	printf("---> thread #%zu\n", i);

	if (i == 0) {
		print_ids();
		return NULL;
	}

	/*
	 * Serialize all threads to be run in descending order:
	 * Every following thread apart of last, creates the child one.
	 */
	err = pthread_create(&thr_ids[i-1], NULL, thread_func,
			     (void *)(i-1));
	if (err) {
		fprintf(stderr, "Error: %s (id=%zu): pthread_create(): %d\n",
			__func__, i, err);
		finished = true;
		return (void *)2;
	}

	print_ids();

	err = pthread_join(thr_ids[i-1], (void **)ret);
	if (err) {
		fprintf(stderr, "Error: %s (id=%zu): pthread_join(): %d\n",
			__func__, i, err);
		ret = 3;
	}

	return (void *)ret; /* propagate error code further */
}

int main(void)
{
	int err, ret = EXIT_SUCCESS;
	unsigned long thr_ret;

	pthread_mutex_init(&lock, NULL);

	err = pthread_create(&thr_ids[THR_NUM - 1], NULL, thread_func,
			     (void *)(THR_NUM - 1));
	if (err) {
		perror("Warning: Error in pthread_create()");
		ret = EXIT_FAILURE;
		goto exit;
	}

	msleep(100);
	finished = true;

	err = pthread_join(thr_ids[THR_NUM-1], (void **)&thr_ret);
	if (err) {
		fprintf(stderr, "Error: pthread_join(), err=%d\n", err);
		ret = EXIT_FAILURE;
		goto exit;
	}
	if (thr_ret) {
		fprintf(stderr, "Error: thread func returned error %lu\n",
			thr_ret);
		ret = EXIT_FAILURE;
	}

exit:
	pthread_mutex_destroy(&lock);
	return ret;
}
