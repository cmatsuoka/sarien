
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "orat.h"

static char msg_buffer[MAX_LEN];

int test_previous;


void test_register_module (struct test_suite *s, TEST_MODULE((*func)), char *name)
{
	struct test_module *m;

	m = (struct test_module *)malloc (sizeof (struct test_module));
	if (m == NULL)
		return;

	m->module = func;
	m->name = strdup(name);
	m->succeeded = 0;
	m->failed = 0;
	m->skipped = 0;

	list_add_tail (&m->list, &s->module_list);

	printf ("*** Registered test module: %s\n", m->name);
}


void test_run_modules (struct test_suite *s)
{
	struct list_head *h;

	time (&s->t0);

	list_for_each (h, &s->module_list, next) {
		struct test_module *m = list_entry(h, struct test_module, list);
		printf ("\n>>> Running module: %s::%s\n", s->name, m->name);
		test_enable (m);
		m->module (s, m);
		printf ("<<< Test results: %d succeeded, %d failed, "
			"%d skipped\n", m->succeeded, m->failed, m->skipped);
	}

	time (&s->t1);
}


struct test_suite *test_initialize (char *name)
{
	struct test_suite *s;

	s = malloc (sizeof (struct test_suite));
	s->name = strdup (name);
	s->succeeded = s->failed = s->skipped = 0;
	s->count = 0;
	INIT_LIST_HEAD(&s->module_list);
	printf ("*** Initialized test suite: %s\n", s->name);

	return s;
}


void test_summarize (struct test_suite *s)
{
	struct list_head *h;
	int total;

	printf ("\n*** Summary for test suite \"%s\"\n\n", s->name);
	printf (" %-23.23s  Success       Fail        Skip\n", "Test module");
	printf (" ---------------------------------------------------------\n");
	list_for_each (h, &s->module_list, next) {
		struct test_module *m = list_entry(h, struct test_module, list);
		total = m->succeeded + m->failed + m->skipped;
		printf (" %-23.23s%3d (%3d%%)  %3d (%3d%%)  %3d (%3d%%)\n",
			m->name,
			m->succeeded, 100 * m->succeeded / total,
			m->failed, 100 * m->failed / total,
			m->skipped, 100 * m->skipped / total);
	}
	printf (" ---------------------------------------------------------\n");
	total = s->succeeded + s->failed + s->skipped;
	printf (" %-23.23s%3d (%3d%%)  %3d (%3d%%)  %3d (%3d%%)\n",
		"TOTAL",
		s->succeeded, 100 * s->succeeded / total,
		s->failed, 100 * s->failed / total,
		s->skipped, 100 * s->skipped / total);

	printf ("\n");
	printf ("*** %s test result: %s\n", s->name,
		s->failed > 0 ? "Failed" : "Passed");
	printf ("*** Elapsed time: %ds\n", (int)(s->t1 - s->t0));

	list_for_each (h, &s->module_list, next) {
		struct test_module *m = list_entry(h, struct test_module, list);
		free (m->name);
		list_del (h);
	}

	free (s->name);
	free (s);
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

void test_enable (struct test_module *module)
{
	module->skip = 0;
}


void test_disable (struct test_module *module, char *reason)
{
	module->skip = 1;
	strcpy (module->skip_reason, reason);
}


void test_prepare (struct test_module *module)
{
	strcpy (msg_buffer, "");
}


void test_execute (struct test_suite *s, struct test_module *m, char *name, test_result i)
{
	switch (test_previous = i) {
	case TEST_OK:
		printf ("+++");
		s->succeeded++;
		m->succeeded++;
		test_report (" ... ok\n");
		break;
	case TEST_FAIL:
		printf ("xxx");
		s->failed++;
		m->failed++;
		test_report (" ... FAILED\n");
		break;
	case TEST_SKIP:
		printf ("---");
		s->skipped++;
		m->skipped++;
		test_report ("[%s] ... SKIPPED\n", m->skip_reason);
		break;
	default:
		abort();
	}

	printf (" %03d: [%s] %s", ++s->count, name, msg_buffer);
}

