adjtimex-test
=============

Sample programs for testing adjtimex fnction.

drift
--------
Test requested adjustment and real adjustment of frequency using adjtimex.

You must have privilegs to adjust time - CAP_SYS_TIME - e.g. root.

Measurement is just informative. On my computer, it results this

    adjustment =    0 ppm -> measured drift =    0.093 ppm
    adjustment =  100 ppm -> measured drift =  100.008 ppm

which seem, that it works. Running program with one parameter with value 512.

    adjustment =    0 ppm -> measured drift =    0.100 ppm
    adjustment =  512 ppm -> measured drift =  499.661 ppm

This shows that maximum allowed adjustment of adjtimex on my computer is 500 ppm.


granularity
-----------
Test clock_gettime and reads time value, until it changes.

Results on my PC are

	granularity:
		clock_gettime: <=628 ns
		gettimeofday: 1000 ns



