
#include <stdio.h>
#include <string.h>
#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "opcodes.h"
#include "list.h"

#define MAX_LEN 1024

#define TEST_FAIL	0
#define TEST_OK		1
#define TEST_SKIP	2

struct test_suite {
	struct list_head list;
	char *name;
	void (*suite)(struct test_suite *);
	int skip;
	char skip_reason[MAX_LEN];
	int skipped;
	int succeeded;
	int failed;
};

#define TEST(n,x) do { test_prepare(suite); test_result(n, suite, suite->skip ? TEST_SKIP : (x)); } while (0)
#define TEST_SUITE(x) void x(struct test_suite *suite)

extern int skip;

void	test_name	(char *);
void	test_prepare	(struct test_suite *);
void	test_result	(char *, struct test_suite *, int);
void	test_say	(char *);
int	test_load_game	(char *);
void	test_disable	(struct test_suite *, char *);
void	test_enable	(struct test_suite *);
int	test_report	(char *fmt, ...);

TEST_SUITE(test_format);
TEST_SUITE(test_arith);
TEST_SUITE(test_flag);
TEST_SUITE(test_picture);


