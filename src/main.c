#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define THR_NUM 10

static pthread_t th_id[THR_NUM];
static bool run[THR_NUM];
static bool finished;
static pthread_mutex_t lock;
static pthread_mutex_t cond_lock;
static pthread_cond_t cond;

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
	size_t thr_id = *(int *)data;

	if (thr_id == 0 && !run[9]) {
		pthread_mutex_lock(&cond_lock);
		pthread_cond_wait(&cond, &cond_lock);
		pthread_mutex_unlock(&cond_lock);
	}

	printf("---> thread #%zu\n", thr_id);
	run[thr_id] = true;

	if (thr_id == 9) {
		pthread_mutex_lock(&cond_lock);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&cond_lock);
	}

	while (!finished)
		printf("%d\n", gen_id());

	return NULL;
}

int main(void)
{
	int err, ret = EXIT_SUCCESS;
	size_t i;
	size_t *thr_args;

	thr_args = malloc(THR_NUM * sizeof(size_t));

	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&cond_lock, NULL);
	pthread_cond_init(&cond, NULL);

	for (i = 0; i < THR_NUM; ++i) {
		thr_args[i] = i;
		err = pthread_create(&th_id[i], NULL, thread_func,
				     &thr_args[i]);
		if (err) {
			perror("Error in pthread_create()");
			ret = EXIT_FAILURE;
			goto err;
		}
	}

	msleep(100);
	finished = true;

	for (i = 0; i < THR_NUM; ++i) {
		err = pthread_join(th_id[i], NULL);
		if (err) {
			perror("Warning: Error in pthread_join()");
			ret = EXIT_FAILURE;
		}
	}

err:
	free(thr_args);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&cond_lock);
	pthread_mutex_destroy(&lock);
	return ret;
}
