#include "test.h"


static int c (char *test, char *expected)
{
	char result[MAX_LEN];
	int match;

	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
	test_report ("%s => %s", test, result);
	if (!(match = !strcmp (result, expected))) {
		test_report (" [Expected: %s]", expected);
	}

	return match;
}


TEST_SUITE(test_format)
{
	setvar (2,2);
	setvar (42,42);
	setvar (142,142);

	TEST("literal", c("Sanity check.", "Sanity check."));
	TEST("escapes", c("\\a\\b\\c 100\\% \\\\ \\%v0", "abc 100% \\ %v0"));
	TEST("variable", c("%v2 %v42 %v142", "2 42 142"));
	TEST("fields", c("%v2|1 %v2|0 %v2|3 %v42|4 %v142|2", "2  002 0042 42"));

	/*
	 * load template game resources
	 */
	if (test_load_game ("template")) {
		test_say ("look at this test");
		strcpy (game.strings[1], "a 100% %01 string test");
	} else {
		test_disable (suite, "needs template game");
	}

	setvar (13,13);
	setvar (14,14);
	setvar (15,15);

	TEST("object", c("(%02)", "(test object)"));
	TEST("texts", c("(%g28)", "(I don't understand \"%w3\")"));
	TEST("words", c("%w1 %w2 %w3", "look test "));
	TEST("strings", c("%s1", "a 100% %01 string test"));
	TEST("message", c("%m27", "I don't understand \"test\""));
	TEST("message", c("(%m25)", "( 13:14:15 )"));
}
