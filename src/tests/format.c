#include "orat.h"
#include "test.h"


static int load_game (char *s)
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


static void say (char *s)
{
	char input[40];

	strcpy (input, s);
	dictionary_words (input);
}


static test_result c (char *test, char *expected)
{
	char result[MAX_LEN];
	int match;

#ifdef HAVE_SNPRINTF
	snprintf (result, MAX_LEN, "%s", agi_sprintf (test));
#else
	sprintf (result, "%s", agi_sprintf (test));
#endif
	test_report ("%s => %s", test, result);
	if (!(match = !strcmp (result, expected))) {
		test_report (" [Expected: %s]", expected);
	}

	return match ? TEST_OK : TEST_FAIL;
}


TEST_MODULE(test_format)
{
	setvar (2,2);
	setvar (42,42);
	setvar (142,142);
	setvar (242,0);

	TEST("literal", c("Sanity check.", "Sanity check."));
	TEST("escapes", c("\\a\\b\\c 100\\% \\\\ \\%v0", "abc 100% \\ %v0"));
	TEST("variable", c("%v2 %v42 %v142", "2 42 142"));
	TEST("fields", c("%v2|1 %v2|0 %v2|3 %v42|4 %v142|2", "2  002 0042 42"));
	TEST("field pad", c("%v242 %v242|1 %v242|2 %v242|3", "0 0 00 000"));

	/*
	 * load template game resources
	 */
	if (load_game ("template")) {
		strcpy (game.strings[1], "a 100% %01 string test");
		say ("look at this test");
	} else {
		test_disable (module, "needs template game");
	}

	TEST("object", c("(%02)", "(test object)"));
	TEST("texts", c("(%g28)", "(I don't understand \"%w3\")"));
	TEST("strings", c("%s1", "a 100% %01 string test"));
	TEST("words", c("%w1 %w2 %w3", "look test "));
	TEST("message", c("%m27", "I don't understand \"test\""));

	setvar (11,11);
	setvar (12,12);
	setvar (13,13);

	TEST("message", c("(%m25)", "( 13:12:11 )"));
}
