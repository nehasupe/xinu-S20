#include <xinu.h>
#include <stdio.h>
/*------------------------------------------------------------------------
 *  * xhs_hello - write argument strings to say hello
 *   *------------------------------------------------------------------------
 *    */
shellcmd xsh_hello(int nargs, char *args[])
{
		/* nargs is number of arguments passed through commandline 
		 * 	and args is the array of arrguments passed*/
		if (nargs == 2) {
			printf("Hello %s, Welcome to the world of Xinu!!\n", args[1]);
		}
		else {
			if (nargs > 2){
				fprintf(stderr, "Too many arguments\n");
			}
			if (nargs < 2){
				fprintf(stderr, "Too few arguments\n");
			}
		return SYSERR;
		}

		return 0;
}




