#include "test.h"


static int check (int cmd, char *test, char *res, int expected, UINT8 *p)
{
	char result[MAX_LEN];
	int numres;
	int match;

	num_tests++;

	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
	printf ("%03d: [%s (%d)] %s = ", num_tests,
		logic_names_cmd[cmd].name, cmd, result);
	execute_agi_command (cmd, p);
	snprintf (result, MAX_LEN, "%s", agi_sprintf (res));
	printf ("%s : ", result);

	numres = strtoul (result, NULL, 0);

	if ((match = (expected == numres))) {
		printf ("ok\n");
	} else {
		printf ("**FAILED** (Expected: %d)\n", expected);
		num_failed++;
	}

	return match;
}

void test_arith ()
{
	UINT8 p[10];

	printf ("\n=== arithmetic operation tests\n");

	setvar (200, 200); sprintf (p, "%c%c", 200, 254);
	check (3, "%%v200(%v200):=254", "%v200", 254, p);
	check (1, "%v200++", "%v200", 255, p);
	check (1, "%v200++", "%v200", 255, p);

	setvar (201, 1); sprintf (p, "%c%c", 200, 201);
	check (4, "%%v200(%v200):=%%v201(%v201)", "%v200", 1, p);
	check (2, "%v200--", "%v200", 0, p);
	check (2, "%v200--", "%v200", 0, p);
}
