#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define THR_NUM 10

static pthread_t th_id[THR_NUM];
size_t i = THR_NUM - 1;
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

/* Function to be operated by thread */
static void *thread_func(void *data)
{
	int err;
	size_t thr_id = (size_t)data;

	printf("---> thread #%zu\n", thr_id);

	/*
	 * Serialize all threads to be run in descending order:
	 * Every following thread apart of last, creates the child one.
	 */

	pthread_mutex_lock(&lock);
	--i;
	pthread_mutex_unlock(&lock);

	if (thr_id) {
		err = pthread_create(&th_id[i], NULL,  thread_func, (void *) i);
		if (err)
			perror("Warning: Error in pthread_create()");
	}

	while (!finished)
		printf("%d\n", gen_id());

	pthread_join(th_id[i], NULL);

	return NULL;
}

int main(void)
{
	int err, ret = EXIT_SUCCESS;

	pthread_mutex_init(&lock, NULL);

	err = pthread_create(&th_id[i], NULL, thread_func, (void *)i);
	if (err) {
		perror("Warning: Error in pthread_create()");
		ret = EXIT_FAILURE;
		goto err;
	}

	msleep(100);
	finished = true;

	err = pthread_join(th_id[THR_NUM-1], NULL);
	if (err) {
		perror("Warning: Error in pthread_join()");
		ret = EXIT_FAILURE;
	}

err:
	pthread_mutex_destroy(&lock);
	return ret;
}
