#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define TH_NUM 10

static pthread_t th_id[TH_NUM];
static bool finished;

/* Function to generate unique ids */
static int gen_id(void)
{
	static int id;
	return id++; /* RMW, non-atomic */
}

/* Function to be operated by thread */
static void *thread_func(void *data)
{
	UNUSED(data);

	while (!finished)
		printf("%d\n", gen_id());

	return NULL;
}

int main(void)
{
	int err;
	size_t i;

	for (i = 0; i < TH_NUM; ++i) {
		err = pthread_create(&th_id[i], NULL, thread_func, &i);
		if (err) {
			perror("Error in pthread_create()");
			return EXIT_FAILURE;
		}
	}

	msleep(100);
	finished = true;

	for (i = 0; i < TH_NUM; ++i) {
		err = pthread_join(th_id[i], NULL);
		if (err) {
			perror("Error in pthread_join()");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
