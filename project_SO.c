#include <stdio.h>
#include <stdlib.h>

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}

void procedure1(int start_time, int work_time, void (f)(int, int) ) {
    int start_time_IO, work_time_IO, endTime, endTime_IO;
    printf("P1 processing : %d\n", start_time );
    printf("P1 work time : %d\n", work_time );

    endTime = start_time + work_time;
    printf("End time : %d\n", endTime );

    start_time_IO = rand_interval(start_time, endTime);

    endTime_IO = start_time_IO - start_time + work_time;
    printf("End time IO : %d\n", endTime_IO );

    work_time_IO = rand_interval(start_time_IO, endTime_IO);
    (f)(start_time_IO,work_time_IO);
}

void printerIO(int start_time, int work_time){
  printf("IO processing : %d\n", start_time );
  printf("IO work time : %d\n", work_time );

}

int main(){

  procedure1(1, 4, printerIO);

  return(0);
}
