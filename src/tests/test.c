
#include <time.h>
#include "orat.h"
#include "test.h"

struct sarien_options opt;
struct agi_game game;

#if (!defined(_TRACE) && !defined(__GNUC__))
INLINE void _D (char *s, ...) { s = s; }
#endif


int st_load_game (char *s)
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


void st_say (char *s)
{
	char input[40];

	strcpy (input, s);
	dictionary_words (input);
}


int main (int argc, char **argv)
{
	time_t t;
	TEST_SUITE(sarien);

	game.sbuf = malloc (_WIDTH * _HEIGHT);
	game.hires = malloc (_WIDTH * 2 * _HEIGHT);

	printf ("Sarien %s tests (build %s %s)\n", VERSION, __DATE__, __TIME__);
	printf ("Current time: %s\n", ctime (&t));

	sarien = test_initialize ("Sarien");

	test_register_module (sarien, test_flag, "flag operations");
	test_register_module (sarien, test_arith, "arithmetic operations");
	test_register_module (sarien, test_format, "string formatting");
	test_register_module (sarien, test_picture, "picture drawing");

	test_run_modules (sarien);

	test_summarize (sarien);

	free (game.sbuf);
	free (game.hires);

	return 0;
}

