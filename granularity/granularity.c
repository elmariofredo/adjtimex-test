#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

/*
 * Test clock_gettime precision
 * 
 */

int32_t
diff_timespec(struct timespec * a, struct timespec * b) {
	return (b->tv_sec - a->tv_sec)*1000000000 + (b->tv_nsec - a->tv_nsec);
}

int32_t
diff_timeval(struct timeval * a, struct timeval * b) {
	return (b->tv_sec - a->tv_sec)*1000000000 + (b->tv_usec - a->tv_usec)*1000;
}

void
measure_clock_gettime(void) {
	struct timespec ta;
	struct timespec tb;
	
	int32_t granularity;
	int cnt = 0;
	
	clock_gettime(CLOCK_REALTIME, &ta);
	tb = ta;
	while(tb.tv_nsec == ta.tv_nsec) {
		clock_gettime(CLOCK_REALTIME, &ta);
		cnt++;
	}
	
	granularity = diff_timespec(&tb, &ta);
	
	printf("\tclock_gettime: %s%d ns\r\n", cnt == 1 ? "<=" : "", granularity);
}

void
measure_gettimeofday(void) {
	struct timeval ta;
	struct timeval tb;
	
	int32_t granularity;
	int cnt = 0;
	
	gettimeofday(&ta, 0);
	tb = ta;
	while(tb.tv_usec == ta.tv_usec) {
		gettimeofday(&ta, 0);
		cnt++;
	}
	
	granularity = diff_timeval(&tb, &ta);
	
	printf("\tgettimeofday: %s%d ns\r\n", (cnt == 1 && granularity != 1000) ? "<=" : "", granularity);
}

int
main(int argc, char ** argv) {

	printf("granularity:\r\n");
	measure_clock_gettime();
	measure_gettimeofday();
	
	return 0;
}