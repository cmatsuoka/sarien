#include "orat.h"
#include "test.h"
#include "rand.h"


static test_result dist()
{
	int i;
	UINT8 p[10];
	int min, x[256];

	test_report ("Randomness test (5000x)");

	for (i = 0; i < 256; i++) {
		x[i] = 0;
	}

	sprintf (p, "%c%c%c", 0, 255, 100);
	for (i = 0; i < 5000; i++) {
		execute_agi_command (130, p);
		x[getvar(100)]++;
	}
		
	min = 5000;
	for (i = 0; i < 256; i++) {
		if (x[i] < min)
			min = x[i];
		if (x[i] == 0)
			return TEST_FAIL;
	}

	test_report (" => min %d", min);

	return TEST_OK;
}

TEST_MODULE(test_random)
{
	set_rnd_seed ();

	TEST ("distribution", dist());
}

