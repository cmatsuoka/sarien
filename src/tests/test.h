#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "opcodes.h"

#define MAX_LEN 1024

#define TEST_FAIL	0
#define TEST_OK		1
#define TEST_SKIP	2

#define TEST(x)	do { test_count(); test_result(skip?TEST_SKIP:(x)); } while (0)

extern int skip;

void	test_name	(char *);
void	test_count	(void);
void	test_result	(int);
void	test_say	(char *);
int	test_load_game	(char *);
void	test_disable	(char *);
void	test_enable	(void);

void	test_format	(void);
void	test_arith	(void);



