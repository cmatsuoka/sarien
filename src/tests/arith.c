#include "orat.h"
#include "test.h"


static test_result c (int cmd, char *test, char *res, int expected, UINT8 *p)
{
	char result[MAX_LEN];
	int numres;
	int match;

	sprintf (result, "%s", agi_sprintf (test));
	test_report ("(%d)%s %s = ", cmd, logic_names_cmd[cmd].name, result);
	execute_agi_command (cmd, p);
	sprintf (result, "%s", agi_sprintf (res));
	test_report ("%s", result);

	numres = strtoul (result, NULL, 0);

	if (!(match = (expected == numres))) {
		test_report (" [Expected: %d]", expected);
	}

	return match ? TEST_OK : TEST_FAIL;
}


TEST_MODULE(test_arith)
{
	UINT8 p[10];

	setvar (200, 200); sprintf (p, "%c%c", 200, 254);
	TEST ("assignment", c(3, "v200(%v200):=254", "%v200", 254, p));
	TEST ("increment", c(1, "%v200++", "%v200", 255, p));
	TEST ("increment rollover", c(1, "%v200++", "%v200", 255, p));

	setvar (201, 1); sprintf (p, "%c%c", 200, 201);
	TEST ("assignment", c(4, "v200(%v200):=v201(%v201)", "%v200", 1, p));
	TEST ("decrement", c(2, "%v200--", "%v200", 0, p));
	TEST ("decrement rollover", c(2, "%v200--", "%v200", 0, p));

	/* addn */
	setvar (200, 250); sprintf (p, "%c%c", 200, 3);
	TEST ("addition", c(5, "%v200 + 3", "%v200", 253, p));
	TEST ("addition rollover", c(5, "%v200 + 3", "%v200", 0, p));

	/* addv, subn, subv */
	setvar (201, 129); sprintf (p, "%c%c", 200, 201);
	TEST ("addition", c(6, "%v200 + %v201", "%v200", 129, p));
	TEST ("addition rollover", c(6, "%v200 + %v201", "%v200", 2, p));
	TEST ("subtraction rollover", c(8, "%v200 - %v201", "%v200", 129, p));
	TEST ("subtraction", c(7, "%v200 - 201", "%v200", 184, p));
	
	/* mul.n, div.n */
	setvar (200, 125); sprintf (p, "%c%c", 200, 2);
	TEST ("multiplication", c(165, "%v200 * 2", "%v200", 250, p));
	TEST ("multiplication rollover", c(165, "%v200 * 2", "%v200", 244, p));
	TEST ("division", c(167, "%v200 / 2", "%v200", 122, p));

	/* mul.v, div.v */
	setvar (201, 100); sprintf (p, "%c%c", 200, 201);
	TEST ("division", c(168, "%v200 / %v201", "%v200", 1, p));
	TEST ("multiplication", c(166, "%v200 * %v201", "%v200", 100, p));

	/* indirections */
	setvar (201, 200); sprintf (p, "%c%c", 201, 42);
	TEST ("indirection", c(11, "v[v201(%v201)](%v200):=42", "%v200", 42, p));

	setvar (201, 200); sprintf (p, "%c%c", 201, 201);
	TEST ("indirection", c(9, "v[v201(%v201)](%v200):=v201(%v201)","%v200",200,p));

	setvar (201, 50); setvar (50, 50); sprintf (p, "%c%c", 200, 201);
	TEST ("indirection", c(10, "v200(%v200):=v[v201(%v201)](%v50)","%v200",50,p));
}

