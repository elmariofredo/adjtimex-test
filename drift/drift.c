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
#define ADJ_FREQ_MAX 500000
#define servoMaxPpb (ADJ_FREQ_MAX*100)
#define Integer32 int32_t
#define Boolean int

void
adjFreq(Integer32 adj)
{
    struct timex t;
    Integer32 tickAdj = 0;
    int result;

#ifdef RUNTIME_DEBUG
    Integer32 oldAdj = adj;
#endif

    memset(&t, 0, sizeof(t));

    /* Get the USER_HZ value */
    Integer32 userHZ = sysconf(_SC_CLK_TCK);

    /*
     * Get the tick resolution (ppb) - offset caused by changing the tick value by 1.
     * The ticks value is the duration of one tick in us. So with userHz = 100  ticks per second,
     * change of ticks by 1 (us) means a 100 us frequency shift = 100 ppm = 100000 ppb.
     * For userHZ = 1000, change by 1 is a 1ms offset (10 times more ticks per second)
     */
    Integer32 tickRes = userHZ * 1000;

    /* Clamp to max PPM */
    if (adj > servoMaxPpb){
        adj = servoMaxPpb;
    } else if (adj < -servoMaxPpb){
        adj = -servoMaxPpb;
    }

    /*
     * If we are outside the standard +/-512ppm, switch to a tick + freq combination:
     * See how many ticks we are above 512ppm, turn that into ticks and add / subtract
     * those ticks (converted to ppb) from the freq adjustments. This is ceil'd so we will
     * only adjust if it's enough for a tick. The offset change will not be smooth as we flip
     * between tick and frequency, but this in general should only be happening under extreme
     * conditions when dragging the offset down from very large values. When maxPPM is left at
     * the default value, behaviour is the same as previously, clamped to 512ppm, but we keep
     * tick at the base value, preventing long stabilisation times say when  we had a non-default
     * tick value left over from a previous NTP run.
     *
     * With this in place, if our adj is say between 512 and 612 ppm, adj will be stuck at
     * 512 with no tick adjustment. This is intentional - if we used residuals, we would
     * keep flapping between tick and freq which we don't want.
     */
    if (adj > ADJ_FREQ_MAX){
        tickAdj = -  ((long)ceil(( (adj + 0.0)  - ADJ_FREQ_MAX) / (tickRes + 0.0)) );
        adj += tickAdj * tickRes;
    } else if (adj < -ADJ_FREQ_MAX){
        tickAdj = (long)ceil(( abs(adj + 0.0)  - ADJ_FREQ_MAX) / (tickRes + 0.0)) ;
        adj += tickAdj * tickRes;
        }
    /* Base tick duration - 10000 when userHZ = 100 */
    t.tick = 1E6 / userHZ;
    /* Tick adjustment if necessary */
    t.tick -= tickAdj;

    t.modes = MOD_FREQUENCY | ADJ_TICK;
    t.freq = adj * ((1 << 16) / 1000);

    /* do calculation in double precision, instead of Integer32 */
    int t1 = t.freq;
    int t2;
    
    float f = (adj + 0.0) * (((1 << 16) + 0.0) / 1000.0);  /* could be float f = adj * 65.536 */
    t2 = t1;  // just to avoid compiler warning
    t2 = (int)round(f);
    t.freq = t2;
//    DBG2("adjFreq: oldadj: %d, newadj: %d, tick: %d, tickadj: %d\n", oldAdj, adj,t.tick,tickAdj);
//    DBG2("        adj is %d;  t freq is %d       (float: %f Integer32: %d)\n", adj, t.freq,  f, t1);
    
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