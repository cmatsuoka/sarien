#ifndef __ORAT_H
#define __ORAT_H

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef INLINE
#  define INLINE
#endif

#include "list.h"

#define MAX_LEN 1024

typedef enum test_result {
	TEST_FAIL,
	TEST_OK,
	TEST_SKIP
} test_result;

struct test_module;
struct test_suite;

#define TEST(n,x) do { \
	test_prepare(module); \
	test_execute(suite, module, n, module->skip ? TEST_SKIP : (x)); \
} while (0)

#define TEST_MODULE(x) \
	void x(struct test_suite *suite, struct test_module *module)

#define TEST_SUITE(s) struct test_suite *s


struct test_suite {
	char *name;
	int count;
	int skipped;
	int succeeded;
	int failed;
	time_t t0, t1;
	struct list_head module_list;
};

struct test_module {
	struct list_head list;
	char *name;
	TEST_MODULE((*module));
	int skip;
	char skip_reason[MAX_LEN];
	int skipped;
	int succeeded;
	int failed;
};


struct test_suite *
	test_initialize		(char *name);
void	test_register_module	(struct test_suite *, TEST_MODULE((*func)), char *);
void	test_run_modules	(struct test_suite *);
void	test_summarize		(struct test_suite *);
void	test_prepare		(struct test_module *);
void	test_execute		(struct test_suite *, struct test_module *, char *, test_result);
void	test_disable		(struct test_module *, char *);
void	test_enable		(struct test_module *);
int	test_report		(char *, ...);

extern int test_previous;


#endif /* __ORAT_H */
