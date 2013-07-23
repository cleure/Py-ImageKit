
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <pthread.h>
#include <assert.h>

#include "imagekit.h"
#include "hashtable.h"
#include "tests/framework.h"

void *thread1(void *arg)
{
    int i;
    int e_code;
    char *e_msg;
    
    char *test_msgs[] = {"Error 1", "Error 2", "Error 3", "Error 4"};
    int test_codes[] = {1, 2, 3, 4};
    
    int rnd = rand() % 4;
    for (i = 0; i < 0xfff; i++) {
        if (!ImageKit_SetError(test_codes[rnd], test_msgs[rnd])) {
            printf("Warning: ImageKit_SetError() Failed...\n");
            return NULL;
        }
    }
    
    for (i = 0; i < 0xff; i++) {
        ImageKit_LastError(&e_code, &e_msg);
        assert(e_code == test_codes[rnd]);
        assert(strcmp(test_msgs[rnd], e_msg) == 0);
    }
    
    return NULL;
}

void *thread2(void *arg)
{
    int i;
    int e_code;
    char *e_msg;
    
    char *test_msgs[] = {"Error 5", "Error 6", "Error 7", "Error 8"};
    int test_codes[] = {5, 6, 7, 8};
    
    int rnd = rand() % 4;
    for (i = 0; i < 0xff; i++) {
        if (!ImageKit_SetError(test_codes[rnd], test_msgs[rnd])) {
            printf("Warning: ImageKit_SetError() Failed...\n");
            return NULL;
        }
    }
    
    for (i = 0; i < 0xfff; i++) {
        ImageKit_LastError(&e_code, &e_msg);
        assert(e_code == test_codes[rnd]);
        assert(strcmp(test_msgs[rnd], e_msg) == 0);
    }
    
    return NULL;
}

int main(void)
{
    int i;
    pthread_t threads[16];
    
    /* Test errors with threading */
    for (i = 0; i < 8; i++) {
        pthread_create(&threads[i*2], NULL, &thread1, NULL);
        pthread_create(&threads[i*2+1], NULL, &thread2, NULL);
    }
    
    for (i = 0; i < 8; i++) {
        pthread_join(threads[i*2], NULL);
        pthread_join(threads[i*2+1], NULL);
    }

    return 0;
}
