
#include <time.h>
#include "orat.h"
#include "test.h"

struct sarien_options opt;
struct agi_game game;

#if (!defined(_TRACE) && !defined(__GNUC__))
INLINE void _D (char *s, ...) { s = s; }
#endif



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
	test_register_module (sarien, test_random, "random numbers");
	test_register_module (sarien, test_format, "string formatting");
	test_register_module (sarien, test_picture, "picture drawing");
	test_register_module (sarien, test_inventory, "inventory");

	test_run_modules (sarien);

	test_summarize (sarien);

	free (game.sbuf);
	free (game.hires);

	return 0;
}

