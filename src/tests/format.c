#include "test.h"


static int check (char *name, char *test, char *expected)
{
	char result[MAX_LEN];
	int match;

	num_tests++;

	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
	printf ("%03d: [%s] %s => %s : ", num_tests, name, test, result);
	if ((match = !strcmp (result, expected))) {
		printf ("ok\n");
	} else {
		printf ("**FAILED** (Expected: %s)\n", expected);
		num_failed++;
	}

	return match;
}

void test_format ()
{
	printf ("\n=== AGI string formatting tests\n");

	check ("literal", "Sanity check.", "Sanity check.");
	check ("escapes", "\\a\\b\\c 100\\% \\\\ \\%v0", "abc 100% \\ %v0");
	check ("variable", "%v2 %v42 %v142", "2 42 142");
	check ("fields", "%v2|1 %v2|0 %v2|3 %v42|4 %v142|2", "2  002 0042 42");
	check ("object", "(%02)", "(test object)");
	check ("texts", "(%g28)", "(I don't understand \"%w3\")");
	check ("words", "%w1 %w2 %w3", "look test ");
	check ("strings", "%s1", "a 100% %01 string test");
	check ("message", "%m27", "I don't understand \"test\"");
	check ("message", "(%m25)", "( 13:12:11 )");
}
