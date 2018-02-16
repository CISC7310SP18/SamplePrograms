#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#include "worker.h"

/*
 * modeled after the example program pthread_create(2)
 */
int main(int argc, char *argv[]) {
    long long totalaccepted = 0, trials = 0;
    int tnum, num_threads, status;
    struct thread_info *tinfo;
    pthread_attr_t attr;
    void *res;
    double pi;

    /* get parameters from the command line */
    if (argc > 1) {
        sscanf(argv[1], "%d", &num_threads);
    } else {
        printf("Usage: %s <NUMBER_OF_PROCESSES> <P1_MAX_ITERS> <P1_SEED_X> <P1_SEED_Y> <...>\n"
                , argv[0]);
        exit(EXIT_SUCCESS);
    }

    if (argc < num_threads*3+2) {
        printf("Usage: %s <NUMBER_OF_PROCESSES> <P1_MAX_ITERS> <P1_SEED_X> <P1_SEED_Y> <...>\n"
                , argv[0]);
        exit(EXIT_SUCCESS);
    }

    tinfo = calloc(num_threads, sizeof(struct thread_info));
    if (tinfo == NULL) {
        handle_error("calloc");
    }

    for (tnum=0; tnum<num_threads; tnum++) {
        sscanf(argv[2+3*tnum], "%lld", &(tinfo[tnum].maxiter)); 
        sscanf(argv[2+3*tnum+1], "%d", &(tinfo[tnum].seedx));
        sscanf(argv[2+3*tnum+2], "%d", &(tinfo[tnum].seedy));
    }

    for (tnum=0; tnum<num_threads; tnum++) {
        printf("Read from the command line for thread(%d): %lld %d %d\n", tnum
                , tinfo[tnum].maxiter , tinfo[tnum].seedx, tinfo[tnum].seedy);
    }

    /* run threads: multiple steps */
    /* Initialize thread creation attributes */
    status = pthread_attr_init(&attr);
    if (status != 0) {
        handle_error_en(status, "pthread_attr_init");
    }


    /* Create one thread for set of parameters */
    for (tnum = 0; tnum < num_threads; tnum++) {
        tinfo[tnum].thread_num = tnum;

        /* The pthread_create() call stores the thread ID into corresponding element 
         * of tinfo[] */
        status = pthread_create(&tinfo[tnum].thread_id, &attr, &piworker, &tinfo[tnum]);
        if (status != 0) {
            handle_error_en(status, "pthread_create");
        }
    }

    /* Destroy the thread attributes object, since it is no longer needed */
    status = pthread_attr_destroy(&attr);
    if (status != 0) {
        handle_error_en(status, "pthread_attr_destroy");
    }

    /* Now join with each thread, and display its returned value */
    for (tnum = 0; tnum < num_threads; tnum++) {
        status = pthread_join(tinfo[tnum].thread_id, &res);
        if (status != 0) {
            handle_error_en(status, "pthread_join");
        }

        printf("Joined with thread %d; returned value was %lf\n", tinfo[tnum].thread_num, 
            *((double *)res));
        free(res);      /* Free memory allocated by thread */
    }

    for (tnum = 0; tnum < num_threads; tnum++) {
        totalaccepted += tinfo[tnum].accepted;
        trials += tinfo[tnum].maxiter;
    }
    printf("max trials = %lld total accepted = %lld\n", trials, totalaccepted);
    pi = (double)totalaccepted / (double)trials * 4.0;
    printf("In parent: estimated pi = %lf\n", pi);

    free(tinfo);
    exit(EXIT_SUCCESS);
}

