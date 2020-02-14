#include <cstdlib>
#include <tbb/parallel_sort.h>
#include <cmath>
#include <sys/time.h>
#include <algorithm>

#define N 655320
float a[N];
float b[N];
struct C
{
    int v1, v2;
    bool operator<(const C &s1) const {
        if(this->v2 < s1.v2)
            return false;
        else
            return true;    
    }
};
C c[N];

long long get_time()
{
    timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 10e6 + time.tv_usec;
}

int main()
{
    long long single_thrd_time, multi_thrd_time;

 
    for( int i = 0; i < N; i++ ) {
        a[i] = sin((double)i);
        b[i] = cos((double)i);
    }
   
    for( int i = 0; i < N; i++ ) {
        c[i].v1 = i;
        c[i].v2 = 1;
    }
    //sort
    single_thrd_time = get_time(); 
    // std::sort(b, b + N);
    single_thrd_time = get_time() - single_thrd_time;

    //parallel sort
    multi_thrd_time = get_time();
    // tbb::parallel_sort(a, a + N);
    multi_thrd_time = get_time() -multi_thrd_time;

    std::sort(c, c + N);
    printf("first: %d\tsecond:%d\n", c[0].v1, c[1].v1);
    printf("single: %lld\n", single_thrd_time);
    printf("multi : %lld\n", multi_thrd_time);
    return 0;
}