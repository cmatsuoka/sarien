#include "test.h"


static int c (int cmd, char *test, char *res, int expected, UINT8 *p)
{
	char result[MAX_LEN];
	int numres;
	int match;

	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
	printf ("[%s (%d)] %s = ", logic_names_cmd[cmd].name, cmd, result);
	execute_agi_command (cmd, p);
	snprintf (result, MAX_LEN, "%s", agi_sprintf (res));
	printf ("%s", result);

	numres = strtoul (result, NULL, 0);

	if (!(match = (expected == numres))) {
		printf (" [Expected: %d]", expected);
	}

	return match;
}

void test_arith ()
{
	UINT8 p[10];

	test_name ("arithmetic operation tests");

	setvar (200, 200); sprintf (p, "%c%c", 200, 254);
	TEST (c(3, "%%v200(%v200):=254", "%v200", 254, p));
	TEST (c(1, "%v200++", "%v200", 255, p));
	TEST (c(1, "%v200++", "%v200", 255, p));

	setvar (201, 1); sprintf (p, "%c%c", 200, 201);
	TEST (c(4, "%%v200(%v200):=%%v201(%v201)", "%v200", 1, p));
	TEST (c(2, "%v200--", "%v200", 0, p));
	TEST (c(2, "%v200--", "%v200", 0, p));

	/* addn */
	setvar (200, 250); sprintf (p, "%c%c", 200, 3);
	TEST (c(5, "%v200 + 3", "%v200", 253, p));
	TEST (c(5, "%v200 + 3", "%v200", 0, p));

	/* addv, subn, subv */
	setvar (201, 129); sprintf (p, "%c%c", 200, 201);
	TEST (c(6, "%v200 + %v201", "%v200", 129, p));
	TEST (c(6, "%v200 + %v201", "%v200", 2, p));
	TEST (c(8, "%v200 - %v201", "%v200", 129, p));
	TEST (c(7, "%v200 - 201", "%v200", 184, p));
	
	/* mul.n, div.n */
	setvar (200, 125); sprintf (p, "%c%c", 200, 2);
	TEST (c(165, "%v200 * 2", "%v200", 250, p));
	TEST (c(165, "%v200 * 2", "%v200", 244, p));
	TEST (c(167, "%v200 / 2", "%v200", 122, p));

	/* mul.v, div.v */
	setvar (201, 100); sprintf (p, "%c%c", 200, 201);
	TEST (c(168, "%v200 / %v201", "%v200", 1, p));
	TEST (c(166, "%v200 * %v201", "%v200", 100, p));

	/* indirections */
	setvar (201, 200); sprintf (p, "%c%c", 201, 42);
	TEST (c(11, "%%v[%%v201(%v201)](%v200):=42", "%v200", 42, p));

	setvar (201, 200); sprintf (p, "%c%c", 201, 201);
	TEST (c(9, "%%v[%%v201(%v201)](%v200):=%%v201(%v201)","%v200",200,p));

	setvar (201, 150); sprintf (p, "%c%c", 200, 201);
	TEST (c(10, "%%v200(%v200):=%%v[%%v201(%v201)](%v150)","%v200",150,p));
}
