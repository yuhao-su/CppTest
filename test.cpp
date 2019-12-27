#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <libaio.h>
#include <sys/time.h>

#define WRITE_SIZE 64 * 1024 * 1024
long long get_time()
{
    timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 10e6 + time.tv_usec;
}
int main(void)
{
    int               output_fd;
    struct iocb       io, *p=&io;
    struct io_event   e;
    // struct timespec   timeout;
    io_context_t      ctx;
    const char        *content;
    content = new char[WRITE_SIZE];
    int event_num;
    long long io_submit_time, io_poll_time;
    // 1. init the io context.
    memset(&ctx, 0, sizeof(ctx));
    if(io_setup(10, &ctx)){
        printf("io_setup error\n");
        return -1;
    }

    // 2. try to open a file.
    if((output_fd=open("foobar.txt", O_CREAT|O_WRONLY, 0644)) < 0) {
        perror("open error");
        io_destroy(ctx);
        return -1;
    }

    // 3. prepare the data.
    io_prep_pwrite(&io, output_fd, (void*)content, WRITE_SIZE, 0);
    //io.data = content;   // set or not
    io_submit_time = get_time();
    if(io_submit(ctx, 1, &p) < 0){
        io_destroy(ctx);
        printf("io_submit error\n");
        return -1;
    }
    io_submit_time = get_time() - io_submit_time;
    // 4. wait IO finish.
    io_poll_time = get_time();
    if((event_num = io_getevents(ctx, 0, 1, &e, NULL))) {
        printf("number of events: %d\n", event_num);
    }
    io_poll_time = get_time() - io_poll_time;

    printf("io submit time: %lld\n", io_submit_time);
    printf("io  poll  time: %lld\n", io_poll_time);
    io_destroy(ctx);
    return 0;
}
