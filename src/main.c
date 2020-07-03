#include <tools.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define TH_NUM 10

/* function to generate unique ids */
int id_gen()
{
	static int id;
	return id++;
}

/* function to be operated by thread */
void *thread_func(void *data)
{
	//UNUSED(data);
	printf("thread %i - id %d\n", *(int *)data, id_gen());
	return NULL;
}

int main(void)
{
	pthread_t th_id[TH_NUM];
	int ret;
	size_t i;
	for(i = 0; i < TH_NUM; ++i) {
		th_id[i] = i;
		ret = pthread_create(&th_id[i], NULL, thread_func, &i);
		if (ret) {
			perror("Error in pthread_create()");
			return EXIT_FAILURE;
		}
	}

	msleep(5000);

	for(i = 0; i < TH_NUM; ++i) {
		ret = pthread_join(th_id[i], NULL);
		if (ret) {
			perror("Error in pthread_join()");
			return EXIT_FAILURE;
		}	
	}
	
	return EXIT_SUCCESS;
}
