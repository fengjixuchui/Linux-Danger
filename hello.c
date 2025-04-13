#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<memory.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<signal.h>

// def a dummy signal handler
void dummy_handler(int signo)
{
	printf("Received signal %d\n", signo);
}

int main(int argc, char **argv)
{
	// read CS, DS, SS
	unsigned int cs, ds, ss;
	asm volatile("mov %%cs, %0" : "=r"(cs));
	asm volatile("mov %%ds, %0" : "=r"(ds));
	asm volatile("mov %%ss, %0" : "=r"(ss));
	printf("Congratulations! We are now in Usermode, CS = 0x%x, DS = 0x%x, SS = 0x%x\n", cs, ds, ss);
	// Let's do some self-checks

	// first, check malloc (small)
	void *ptr = malloc(128);
	printf("malloc(128) = %p\n", ptr);
	// attempt to write something to the memory
	memset(ptr, 0x66, 128);
	printf("Successfully written to the memory\n");
	// free the memory
	free(ptr);
	ptr = 0;
	printf("Successfully freed the memory\n");

	// then, check malloc (large)
	ptr = malloc(1024 * 1024);
	printf("malloc(1024 * 1024) = %p\n", ptr);
	// attempt to write something to the memory
	memset(ptr, 0x66, 1024 * 1024);
	printf("Successfully written to the memory\n");
	// free the memory
	free(ptr);
	ptr = 0;
	printf("Successfully freed the memory\n");

	// then, file operations
	int fd = open(argv[0], O_RDONLY);
	printf("open(%s, O_RDONLY) = %d\n", argv[0], fd);
	if(fd < 0)
	{
		printf("Failed to open the file\n");
		return 1;
	}
	// read the file
	char buf[1024];
	int n = read(fd, buf, 1024);
	printf("read(%d, buf, 1024) = %d\n", fd, n);
	if(n < 0)
	{
		printf("Failed to read the file\n");
		close(fd);
		return 1;
	}
	// mmap
	ptr = mmap(0, 1024, PROT_READ, MAP_PRIVATE, fd, 0);
	printf("mmap(0, 1024, PROT_READ, MAP_PRIVATE, %d, 0) = %p\n", fd, ptr);
	if(ptr < 0)
	{
		printf("Failed to mmap the file\n");
		close(fd);
		return 1;
	}
	// munmap
	munmap(ptr, 1024);
	ptr = 0;
	printf("Successfully munmapped the file\n");
	// close the file
	close(fd);
	printf("Successfully closed the file\n");

	// then, getpid
	pid_t pid = getpid();
	printf("getpid() = %d\n", pid);

	// then, getppid
	pid_t ppid = getppid();
	printf("getppid() = %d\n", ppid);

	// then, getuid
	int uid = getuid();
	printf("getuid() = %d\n", uid);

	// then, regist signal handler
	signal(SIGINT, dummy_handler);
	printf("Successfully registered signal handler for SIGINT\n");
	signal(SIGTERM, dummy_handler);
	printf("Successfully registered signal handler for SIGTERM\n");
	signal(SIGKILL, dummy_handler);
	printf("Successfully registered signal handler for SIGKILL\n");

	// then, test SIMD
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
	);
	printf("SIMD test passed, c = [%d, %d, %d, %d]\n", c[0], c[1], c[2], c[3]);

	printf("Almost passed all the tests\n");
	// then, test scanf
	int num = 0;
	printf("Please input an integer: ");
	scanf("%d", &num);
	printf("You have input: %d\n", num);
}