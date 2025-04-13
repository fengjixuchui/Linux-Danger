#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ptrace.h>

// Signal handler
void dummy_handler(int signo) {
    printf("Received signal %d\n", signo);
}

// Thread function
void *thread_func(void *arg) {
    while (1) {
        printf("[Thread] Running in Usermode, PID: %d, TID: %ld\n", getpid(), pthread_self());
        sleep(2);
    }
}

// Run fork() + execve() test
void fork_exec_test() {
    printf("Starting fork() + execve() test...\n");
    
    pid_t pid = fork();
    if (pid < 0) {
        printf("fork() failed: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        printf("[Child] Hello from child process, PID: %d, Parent PID: %d\n", getpid(), getppid());
        char *args[] = {"/usr/bin/printf", "Hello from execve!", NULL};
        execve(args[0], args, NULL);
        printf("[Child] execve() failed: %s\n", strerror(errno));
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        printf("[Parent] Child process exited with status %d\n", status);
    }
}

// Run mmap() test
void mmap_test() {
    printf("Starting mmap() test...\n");
    void *ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    printf("mmap(4096) = %p\n", ptr);

    if (ptr == MAP_FAILED) {
        printf("mmap() failed: %s\n", strerror(errno));
        return;
    }

    memset(ptr, 0x66, 4096);
    printf("Successfully wrote to mmap()'ed memory.\n");

    if (munmap(ptr, 4096) < 0) {
        printf("munmap() failed: %s\n", strerror(errno));
    } else {
        printf("Successfully unmapped memory.\n");
    }
}

// Test gettimeofday() and clock_gettime()
void time_test() {
    struct timeval tv;
    struct timespec ts;

    if (gettimeofday(&tv, NULL) == 0) {
        printf("gettimeofday() = %ld.%06ld\n", tv.tv_sec, tv.tv_usec);
    } else {
        printf("gettimeofday() failed: %s\n", strerror(errno));
    }

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        printf("clock_gettime() = %ld.%09ld\n", ts.tv_sec, ts.tv_nsec);
    } else {
        printf("clock_gettime() failed: %s\n", strerror(errno));
    }
}

// Test pipes and dup2()
void pipe_test() {
    printf("Starting pipe() test...\n");

    int fd[2];
    if (pipe(fd) == 0) {
        printf("pipe() created: fd[0] = %d, fd[1] = %d\n", fd[0], fd[1]);
        write(fd[1], "Hello, pipe!", 13);
        char buf[16] = {0};
        read(fd[0], buf, 13);
        printf("pipe() read: %s\n", buf);
        close(fd[0]);
        close(fd[1]);
    } else {
        printf("pipe() failed: %s\n", strerror(errno));
    }
}

// Test ptrace()
void ptrace_test() {
    printf("Starting ptrace() test...\n");

    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        printf("ptrace(PTRACE_TRACEME) failed: %s\n", strerror(errno));
    } else {
        printf("ptrace(PTRACE_TRACEME) succeeded.\n");
    }
}

// Test alternative signal stack
void sigaltstack_test() {
    stack_t ss;
    ss.ss_sp = malloc(SIGSTKSZ);
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;

    if (sigaltstack(&ss, NULL) == 0) {
        printf("sigaltstack() succeeded.\n");
    } else {
        printf("sigaltstack() failed: %s\n", strerror(errno));
    }

    free(ss.ss_sp);
}

// Test SIMD instructions
void simd_test() {
    printf("Starting SIMD test...\n");
    unsigned int a[4] = {1, 2, 3, 4};
    unsigned int b[4] = {5, 6, 7, 8};
    unsigned int c[4] = {0};
    asm volatile (
        "movdqa %[a], %%xmm0\n"
        "movdqa %[b], %%xmm1\n"
        "addps %%xmm1, %%xmm0\n"
        "movdqa %%xmm0, %[c]\n"
        : [c] "=m" (c)
        : [a] "m" (a), [b] "m" (b)
        : "%xmm0", "%xmm1"
    );
    printf("Tested movdqa and addps, result: %u %u %u %u\n", c[0], c[1], c[2], c[3]);
}

int main() {
    // Print CPU segment registers
    unsigned int cs, ds, ss;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%ss, %0" : "=r"(ss));
    printf("Running in Usermode, CS = 0x%x, DS = 0x%x, SS = 0x%x\n", cs, ds, ss);

    // Run heap memory tests
    mmap_test();

    // Run multi-threading test
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_func, NULL) == 0) {
        printf("pthread_create() succeeded.\n");
    } else {
        printf("pthread_create() failed: %s\n", strerror(errno));
    }

    // Register signal handlers
    signal(SIGINT, dummy_handler);
    signal(SIGTERM, dummy_handler);

    // Run fork() and execve() test
    fork_exec_test();

    // Run time-related tests
    time_test();

    // Run pipe() and dup2() test
    pipe_test();

    // Run ptrace() test
    ptrace_test();

    // Run sigaltstack() test
    sigaltstack_test();

    // Run SIMD test
    simd_test();

    printf("Exiting main()...\n");
}
