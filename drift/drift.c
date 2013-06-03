#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/timex.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/*
 * Test frequency adjustment - adjtimex
 * 
 */

void
adjFreq(int32_t adj)
{
    struct timex t;
    float f;
    int result;
    
    memset(&t, 0, sizeof(t));

    t.modes = MOD_FREQUENCY;
    
    f = (adj + 0.0) * (((1 << 16) + 0.0) / 1000.0);  /* could be float f = adj * 65.536 */
    t.freq = (int)round(f);
    result = adjtimex(&t);
    
    if(result < 0) {
        printf("Failed to adjtime %d ppm (%s)\r\n", adj, strerror(errno));
        exit(1);
    }
}


static struct timespec ta[4]; 
static struct timespec tb[4];   

#define TIMESTAMP(i) do {                           \
    clock_gettime(CLOCK_REALTIME, &ta[i]);          \
    clock_gettime(CLOCK_MONOTONIC_RAW, &tb[i]);     \
    } while(0)

void
measure(int adj, int slp) {
    adjFreq(0); // set clock frequency adjustment 0ppm
    printf("Measuring %d s with adjustment 0 ppm\r\n", slp);

    TIMESTAMP(0);

    usleep(slp * 1000000); // sleep
    
    TIMESTAMP(1);
    
    adjFreq(adj * 1000); // set clock frequency adjustment "adj" ppm
    printf("Measuring %d s with adjustment %d ppm\r\n", slp, adj);

    TIMESTAMP(2);

    usleep(slp * 1000000); // sleep
    
    TIMESTAMP(3);
 
    adjFreq(0); // set clock frequency adjustment 0ppm
}

double
diff(struct timespec * a, struct timespec * b) {
    return ((b->tv_sec - a->tv_sec)*1000000000.0 + (b->tv_nsec - a->tv_nsec));    
}

void
display(int adj, int slp) {
    double da1, db1, da3, db3;
    
    da1 = diff(&ta[0], &ta[1]) / slp;
    db1 = diff(&tb[0], &tb[1]) / slp;

    da3 = diff(&ta[2], &ta[3]) / slp;
    db3 = diff(&tb[2], &tb[3]) / slp;  
    
    printf("adjustment = %4d ppm -> measured drift = %8.3lf ppm\r\n", 0, (da1-db1)/1000);
    printf("adjustment = %4d ppm -> measured drift = %8.3lf ppm\r\n", adj, (da3-db3)/1000);    
}

int
main(int argc, char ** argv) {

    int adj = 100; // ppm
    int slp = 10; // seconds
    
    if (argc != 3) {
        printf("usage:\r\n\tdrift [adjustment - ppm] [measurement time - seconds]\r\n\r\n");
    }
    
    /* fist parameter: adjustment */
    if (argc > 1) {
        adj = atoi(argv[1]);
    }
    
    /* second parameter: measurement time */
    if (argc > 2) {
        slp = atoi(argv[2]);
    }
   
    measure(adj, slp);
    display(adj, slp);
    
    return 0;
}