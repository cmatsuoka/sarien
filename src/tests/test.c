
#include <time.h>
#include <stdarg.h>
#include "test.h"

struct sarien_options opt;
struct agi_game game;

static int num_succeeded;
static int num_failed;
static int num_skipped;
static int count;
static char msg_buffer[MAX_LEN];

static LIST_HEAD(test_list);



static void new_suite (struct list_head *head, void (*func)(struct test_suite *), char *name)
{
	struct test_suite *s;

	s = (struct test_suite *)malloc (sizeof (struct test_suite));
	if (s == NULL)
		return;

	s->suite = func;
	s->name = strdup(name);
	s->succeeded = 0;
	s->failed = 0;
	s->skipped = 0;

	list_add_tail (&s->list, head);
}


int test_report (char *fmt, ...)
{
	va_list args;
	char buf[MAX_LEN];
	int n;

	va_start (args, fmt);
#ifdef HAVE_VSNPRINTF
	n = vsnprintf (buf, MAX_LEN, fmt, args);
#else
	n = vsprintf (buf, fmt, args);
#endif
	va_end (args);

	strcat (msg_buffer, buf);

	return n;
}


void test_enable (struct test_suite *suite)
{
	suite->skip = 0;
}


void test_disable (struct test_suite *suite, char *reason)
{
	suite->skip = 1;
	strcpy (suite->skip_reason, reason);
}


void test_prepare (struct test_suite *suite)
{
	strcpy (msg_buffer, "");
}


void test_result (char *name, struct test_suite *suite, int i)
{
	switch (i) {
	case TEST_OK:
		printf ("+++");
		num_succeeded++;
		suite->succeeded++;
		test_report (" ... ok\n");
		break;
	case TEST_FAIL:
		printf ("xxx");
		num_failed++;
		suite->failed++;
		test_report (" ... FAILED\n");
		break;
	case TEST_SKIP:
		printf ("---");
		num_skipped++;
		suite->skipped++;
		test_report ("[%s]\t... SKIPPED\n", suite->skip_reason);
		break;
	default:
		abort();
	}

	printf (" %03d: [%s] %s", ++count, name, msg_buffer);
}


int test_load_game (char *s)
{
	int rc = 0;

	rc = (agi_detect_game (s) == err_OK);

	if (rc) {
		load_objects (OBJECTS);
		load_words (WORDS);
		agi_load_resource (rLOGIC, 0);
	}

	return rc;
}


void test_say (char *s)
{
	char input[40];

	strcpy (input, s);
	dictionary_words (input);
}


int main (int argc, char **argv)
{
	time_t t0, t1;
	struct list_head *h;
	int total;

	game.sbuf = malloc (_WIDTH * _HEIGHT);
	game.hires = malloc (_WIDTH * 2 * _HEIGHT);

	time (&t0);

	printf ("Sarien %s tests (build %s %s)\n", VERSION, __DATE__, __TIME__);
	printf ("Current time: %s\n", ctime (&t0));
 
	/*
	 * run our tests!
	 */
	num_succeeded = num_failed = num_skipped = 0;
	count = 0;

	new_suite (&test_list, test_flag, "flag operations");
	new_suite (&test_list, test_arith, "arithmetic operations");
	new_suite (&test_list, test_format, "string formatting");
	new_suite (&test_list, test_picture, "picture drawing");

	list_for_each (h, &test_list, next) {
		struct test_suite *s = list_entry (h, struct test_suite, list);
		printf ("*** Registered test suite: %s\n", s->name);
	}

	list_for_each (h, &test_list, next) {
		struct test_suite *s = list_entry (h, struct test_suite, list);
		printf ("\n>>> Running test: %s\n", s->name);
		test_enable (s);
		s->suite (s);
		printf ("<<< Test results: %d succeeded, %d failed, "
			"%d skipped\n", s->succeeded, s->failed, s->skipped);
	}

	time (&t1);

	printf ("\n*** Summary\n\n");
	printf (" %-23.23s Success        Fail        Skip\n", "Test name");
	printf (" ---------------------------------------------------------\n");
	list_for_each (h, &test_list, next) {
		struct test_suite *s = list_entry (h, struct test_suite, list);
		total = s->succeeded + s->failed + s->skipped;
		printf (" %-23.23s%3d (%3d%%)  %3d (%3d%%)  %3d (%3d%%)\n",
			s->name,
			s->succeeded, 100 * s->succeeded / total,
			s->failed, 100 * s->failed / total,
			s->skipped, 100 * s->skipped / total);
	}
	printf (" ---------------------------------------------------------\n");
	total = num_succeeded + num_failed + num_skipped;
	printf (" %-23.23s%3d (%3d%%)  %3d (%3d%%)  %3d (%3d%%)\n",
		"TOTAL",
		num_succeeded, 100 * num_succeeded / total,
		num_failed, 100 * num_failed / total,
		num_skipped, 100 * num_skipped / total);

	printf ("\n*** Elapsed time: %ds\n", (int)(t1 - t0));

	free (game.sbuf);
	free (game.hires);

	return 0;
}
