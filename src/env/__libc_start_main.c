#include <stdlib.h>

extern void __libc_start_init(void);
extern void __init_libc(char **envp, char *pn);

int __libc_start_main(int (*main)(int,char **,char **), int argc, char **argv)
{
	char **envp = argv+argc+1;

	__init_libc(envp, argv[0]);
	__libc_start_init();

	/* Pass control to the application */
	exit(main(argc, argv, envp));
	return 0;
}
