
#include "test.h"

struct sarien_options opt;
struct agi_game game;

int num_tests;
int num_failed;


int main (int argc, char **argv)
{
	int i;
	char input[40];

	/*
	 * load the standard test game and resources
	 */
	if (agi_detect_game (argv[1]) != err_OK)
		exit (0);
	load_objects (OBJECTS);
	load_words (WORDS);
	agi_load_resource (rLOGIC, 0);

	
	/*
	 * initialize AGI variables
	 */
	for (i = 0; i < MAX_VARS; i++)
		setvar (i, i);
	strcpy (input, "look at this test");
	strcpy (game.strings[1], "a 100% %01 string test");
	setvar (V_word_not_found, 0);
	dictionary_words (input);

	/*
	 * run our tests!
	 */
	num_tests = num_failed = 0;

	test_format ();
	test_arith ();


	printf ("\nNumber of tests: %d\n", num_tests);
	printf ("Failed tests: %d (%3.1f%%)\n", num_failed,
		100.0 * num_failed / num_tests);

	return 0;
}
