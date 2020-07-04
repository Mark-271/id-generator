#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#define TH_NUM 10

static pthread_t th_id[TH_NUM];
static int finished;

/* function to generate unique ids */
static int id_gen(void)
{
	static int id;
	return id++;
}

/* function to be operated by thread */
static void *thread_func(void *data)
{
	UNUSED(data);
	while (!finished)	
		printf("%d\n", id_gen());
	
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

	msleep(5000);
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
