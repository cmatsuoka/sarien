#include "test.h"


static int c (char *name, char *test, char *expected)
{
	char result[MAX_LEN];
	int match;

	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
	printf ("[%s] %s => %s", name, test, result);
	if (!(match = !strcmp (result, expected))) {
		printf (" [Expected: %s]", expected);
	}

	return match;
}


void test_format ()
{
	test_name ("AGI string formatting tests");
	test_enable ();

	setvar (2,2);
	setvar (42,42);
	setvar (142,142);

	TEST(c("literal", "Sanity check.", "Sanity check."));
	TEST(c("escapes", "\\a\\b\\c 100\\% \\\\ \\%v0", "abc 100% \\ %v0"));
	TEST(c("variable", "%v2 %v42 %v142", "2 42 142"));
	TEST(c("fields", "%v2|1 %v2|0 %v2|3 %v42|4 %v142|2", "2  002 0042 42"));

	/*
	 * load template game resources
	 */
	if (test_load_game ("template")) {
		test_say ("look at this test");
		strcpy (game.strings[1], "a 100% %01 string test");
	} else {
		test_disable ("needs template game");
	}

	setvar (13,13);
	setvar (14,14);
	setvar (15,15);

	TEST(c("object", "(%02)", "(test object)"));
	TEST(c("texts", "(%g28)", "(I don't understand \"%w3\")"));
	TEST(c("words", "%w1 %w2 %w3", "look test "));
	TEST(c("strings", "%s1", "a 100% %01 string test"));
	TEST(c("message", "%m27", "I don't understand \"test\""));
	TEST(c("message", "(%m25)", "( 13:14:15 )"));
}
