
#import <Cocoa/Cocoa.h>
#include <stdlib.h>

int	__argc;
char** __argv;

int main(int argc, const char *argv[])
{
	/* For some obscure reasons, when double clicked in the finder, 
	   argc and argv can't be parsed by the GNU's getopt library. */
	__argc = 1;
	__argv = malloc(sizeof(*__argv) * (__argc + 1));
	__argv[0] = strdup("sarien"); 
	__argv[1] = NULL;
	
	if (argv == NULL)
		exit(0);
	
	return NSApplicationMain(argc, argv);
}

