#include "test.h"


static int c (int cmd, int expected)
{
	int i;
	UINT8 p[10];

	test_report ("(%d)%s all flags => ", cmd, logic_names_cmd[cmd].name);

	for (i = 0; i < (MAX_FLAGS * 8); i++) {
		sprintf (p, "%c", i);
		execute_agi_command (cmd, p);

		if (!(expected == getflag(i))) {
			test_report (" [Expected: %d]", expected);
			return TEST_FAIL;
		}
	}

	return TEST_OK;
}

static int d (int cmd, int expected)
{
	int i;
	UINT8 p[10];

	test_report ("(%d)%s all flags => ", cmd, logic_names_cmd[cmd].name);

	for (i = 0; i < (MAX_FLAGS * 8); i++) {
		sprintf (p, "%c", 100);
		setvar (100, i);
		execute_agi_command (cmd, p);

		if (!(expected == getflag(i))) {
			test_report (" [Expected: %d]", expected);
			return TEST_FAIL;
		}
	}

	return TEST_OK;
}

TEST_SUITE(test_flag)
{
	TEST ("set", c(12, TRUE));
	TEST ("reset", c(13, FALSE));
	TEST ("toggle", c(14, TRUE));
	TEST ("toggle", c(14, FALSE));

	TEST ("set.v", d(15, TRUE));
	TEST ("reset.v", d(16, FALSE));
	TEST ("toggle.v", d(17, TRUE));
	TEST ("toggle.v", d(17, FALSE));
}

