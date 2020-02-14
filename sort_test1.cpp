#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <algorithm>

#define N 655320

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


int main()
{
    for( int i = 0; i < N; i++ ) {
        c[i].v1 = i;
        c[i].v2 = 1;
    }

    std::sort(c, c + N);
    printf("first: %d\tsecond:%d\n", c[0].v1, c[1].v1);

    return 0;
}