#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libaio.h>
#include <sys/time.h>
#include <omp.h>

#define WRITE_SIZE 64 * 1024 * 1024
#define READ_SIZE WRITE_SIZE
#define THREAD_NUM 8

long long get_time()
{
    timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 10e6 + time.tv_usec;
}

char* init_buf(int size)
{
    char *buf = (char *)aligned_alloc(4096, size * sizeof(char));
    for (int i = 0; i < WRITE_SIZE; i++)
        buf[i] = 'a';
    return buf;
}

int test_write_block(char* content)
{
    long long io_block_time;
#pragma omp parallel num_threads(THREAD_NUM)
{
    int       output_fd;
    int   nthread  = omp_get_thread_num();
    char* filename = new char[256];
    sprintf(filename, "./output/output_block_%d.txt", nthread);
    if((output_fd=open(filename, O_CREAT|O_WRONLY, 0644)) < 0) {
        perror("open error");
    }

#pragma omp barrier
#pragma omp single
    io_block_time = get_time();

    if(pwrite(output_fd, content, WRITE_SIZE, 0) == -1) {
        printf("pwrite_error!");
    }
    close(output_fd);
#pragma omp barrier
#pragma omp single
{
    io_block_time = get_time() - io_block_time;
    printf("pwrite block time:  %lld\n", io_block_time);
}
}
    
    return 0;
}

int test_read_block()
{
    long long io_block_time;
#pragma omp parallel num_threads(THREAD_NUM)
{
    char* content = init_buf(READ_SIZE);
    int   input_fd;
    int   nthread  = omp_get_thread_num();
    char* filename = new char[256];
    sprintf(filename, "./output/output_block_%d.txt", nthread);
    if((input_fd=open(filename, O_RDONLY, 0644)) < 0) {
        perror("open error");
    }

#pragma omp barrier
#pragma omp single
    io_block_time = get_time();

    if(pread(input_fd, content, READ_SIZE, 0) == -1) {
        printf("pread_error!");
    }
    close(input_fd);
#pragma omp barrier
#pragma omp single
{
    io_block_time = get_time() - io_block_time;
    printf("pread block time:   %lld\n", io_block_time);
}
}
    
    return 0;
}



int test_write_aio(char * content)
{
struct io_event   e[THREAD_NUM]; 
    // struct timespec   timeout;
    io_context_t      ctx;
    int               event_num;
    long long         io_submit_time, io_poll_time;
    
    // 1. init the io context.
    memset(&ctx, 0, sizeof(ctx));
    if(io_setup(10, &ctx)){
        printf("io_setup error\n");
        return -1;
    }

#pragma omp parallel num_threads(THREAD_NUM) 
{
    int          nthread = omp_get_thread_num();
    int          output_fd;
    struct iocb  io, *p=&io;
    char*        filename = new char[256]; 
    sprintf(filename, "./output/output_aio_%d.txt", nthread);
    // 2. try to open a file.
    if((output_fd=open(filename, O_CREAT|O_WRONLY, 0777)) < 0) {
        perror("open error");
        io_destroy(ctx);
    }
    // 3. prepare the data.
    io_prep_pwrite(&io, output_fd, (void*)content, WRITE_SIZE, 0);

    //io.data = content;   // set or not
#pragma omp barrier
#pragma omp single
    io_submit_time = get_time();

    if(io_submit(ctx, 1, &p) < 0){
        io_destroy(ctx);
        printf("io_submit error\n");
    }


}
    io_submit_time = get_time() - io_submit_time;


    // 4. wait IO finish.
    io_poll_time = get_time();
    if((event_num = io_getevents(ctx, 0, THREAD_NUM, e, NULL))) {
        printf("number of events: %d\n", event_num);
    }
    io_poll_time = get_time() - io_poll_time;

    printf("io submit time:     %lld\n", io_submit_time);
    printf("io  poll  time:     %lld\n", io_poll_time);
    io_destroy(ctx);
    return 0;
}

int test_read_aio()
{
struct io_event   e[THREAD_NUM]; 
    // struct timespec   timeout;
    io_context_t      ctx;
    int               event_num;
    long long         io_submit_time, io_poll_time;
    
    // 1. init the io context.
    memset(&ctx, 0, sizeof(ctx));
    if(io_setup(10, &ctx)){
        printf("io_setup error\n");
        return -1;
    }

#pragma omp parallel num_threads(THREAD_NUM) 
{
    char* content = init_buf(READ_SIZE);
    int          nthread = omp_get_thread_num();
    int          input_fd;
    struct iocb  io, *p=&io;
    char*        filename = new char[256]; 
    sprintf(filename, "./output/output_aio_%d.txt", nthread);
    // 2. try to open a file.
    if((input_fd=open(filename, O_RDONLY|O_DIRECT, 0644)) < 0) {
        perror("open error");
        io_destroy(ctx);
    }
    // 3. prepare the data.
    io_prep_pread(&io, input_fd, (void*)content, READ_SIZE, 0);

    //io.data = content;   // set or not
#pragma omp barrier
#pragma omp single
    io_submit_time = get_time();

#pragma omp critical
{
    if(io_submit(ctx, 1, &p) < 0){
        io_destroy(ctx);
        printf("io_submit error\n");
    }
}

}
    io_submit_time = get_time() - io_submit_time;


    // 4. wait IO finish.
    io_poll_time = get_time();
    if((event_num = io_getevents(ctx, 0, THREAD_NUM, e, NULL))) {
        printf("number of events: %d\n", event_num);
    }
    io_poll_time = get_time() - io_poll_time;

    printf("io submit time:     %lld\n", io_submit_time);
    printf("io  poll  time:     %lld\n", io_poll_time);
    io_destroy(ctx);
    return 0;
}

int main(void)
{
    char *content = init_buf(WRITE_SIZE);
    test_write_aio(content);
    test_write_block(content);
    printf("\n");
    // test_read_aio();
    // test_read_block();
    
    return 0;
}
