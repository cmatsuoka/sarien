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

	/* addn */
	setvar (200, 250); sprintf (p, "%c%c", 200, 3);
	check (5, "%v200 + 3", "%v200", 253, p);
	check (5, "%v200 + 3", "%v200", 0, p);

	/* addv, subn, subv */
	setvar (201, 129); sprintf (p, "%c%c", 200, 201);
	check (6, "%v200 + %v201", "%v200", 129, p);
	check (6, "%v200 + %v201", "%v200", 2, p);
	check (8, "%v200 - %v201", "%v200", 129, p);
	check (7, "%v200 - 201", "%v200", 184, p);
	
	/* mul.n, div.n */
	setvar (200, 125); sprintf (p, "%c%c", 200, 2);
	check (165, "%v200 * 2", "%v200", 250, p);
	check (165, "%v200 * 2", "%v200", 244, p);
	check (167, "%v200 / 2", "%v200", 122, p);

	/* mul.v, div.v */
	setvar (201, 100); sprintf (p, "%c%c", 200, 201);
	check (168, "%v200 / %v201", "%v200", 1, p);
	check (166, "%v200 * %v201", "%v200", 100, p);

	/* indirections */
	setvar (201, 200); sprintf (p, "%c%c", 201, 42);
	check (11, "%%v[%%v201(%v201)](%v200):=42", "%v200", 42, p);

	setvar (201, 200); sprintf (p, "%c%c", 201, 201);
	check (9, "%%v[%%v201(%v201)](%v200):=%%v201(%v201)", "%v200", 200, p);

	setvar (201, 50); sprintf (p, "%c%c", 200, 201);
	check (10, "%%v200(%v200):=%%v[%%v201(%v201)](%v50)", "%v200", 50, p);
}
