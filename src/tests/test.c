
#include <unistd.h>
#include <time.h>
#include "test.h"

struct sarien_options opt;
struct agi_game game;

static int num_tests;
static int num_failed;
static int num_skipped;
static int count;
static char skip_reason[256];

int skip;


void test_name (char *s)
{
	printf ("\n=== %s\n\n", s);
	test_enable ();
}


void test_enable ()
{
	skip = 0;
}


void test_disable (char *reason)
{
	skip = 1;
	strcpy (skip_reason, reason);
}


void test_count ()
{
	if (skip)
		num_skipped++;
	else
		num_tests++;

	printf ("%03d: ", ++count);
}


void test_result (int i)
{
	switch (i) {
	case TEST_OK:
		printf ("\t... ok\n");
		break;
	case TEST_FAIL:
		printf ("\t... FAILED\n");
		num_failed++;
		break;
	case TEST_SKIP:
		printf ("[%s]\t... SKIPPED\n", skip_reason);
		break;
	default:
		abort();
	}
}


int test_load_game (char *s)
{
	int fd = 0, rc = 0;

	dup2 (fileno(stderr), fd);
	close (fileno(stderr));

	rc = (agi_detect_game (s) == err_OK);

	if (rc) {
		load_objects (OBJECTS);
		load_words (WORDS);
		agi_load_resource (rLOGIC, 0);
	}

	dup2 (fd, fileno(stderr));

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
	int i;
	time_t t0, t1;

	time (&t0);

	printf ("Sarien %s tests (build %s %s)\n", VERSION, __DATE__, __TIME__);
	printf ("Current time: %s\n", ctime (&t0));
 
	/*
	 * initialize AGI variables
	 */
	for (i = 100; i < MAX_VARS; i++)
		setvar (i, i);

	/*
	 * run our tests!
	 */
	num_tests = num_failed = num_skipped = 0;
	count = 0;

	test_format ();
	test_arith ();

	time (&t1);

	printf ("\n");
	printf ("Skipped tests  : %d\n", num_skipped);
	printf ("Performed tests: %d\n", num_tests);
	printf ("Failed tests   : %d (%3.1f%%)\n", num_failed,
		100.0 * num_failed / num_tests);
	printf ("Elapsed time   : %ds\n", (int)(t1 - t0));

	return 0;
}
