#include "orat.h"
#include "test.h"


static test_result c (int cmd, int obj, int expected, UINT8 *p)
{
	int location;
	int match;

	test_report ("(%d)%s %d => ", cmd, logic_names_cmd[cmd].name, obj);
	execute_agi_command (cmd, p);
	
	location = object_get_location(obj);
	test_report ("new location %d", location);

	if (!(match = (expected == location))) {
		test_report (" [Expected: %d]", expected);
	}

	return match ? TEST_OK : TEST_FAIL;
}


TEST_MODULE(test_inventory)
{
	UINT8 p[10];

	alloc_objects (game.num_objects = 5);

	sprintf (p, "%c", 1);
	TEST ("get", c(0x5c, 1, EGO_OWNED, p));
	TEST ("drop", c(0x5e, 1, 0, p));

	setvar (100, 1); sprintf (p, "%c", 100);
	TEST ("get.v", c(0x5d, 1, EGO_OWNED, p));

	setvar (100, 10); sprintf (p, "%c%c", 1, 100);
	TEST ("put", c(0x5f, 1, 10, p));

	setvar (101, 2); sprintf (p, "%c%c", 101, 100);
	TEST ("put.v", c(0x60, 2, 10, p));

	object_set_location (2,42);
	setvar (100, 2); sprintf (p, "%c%c", 100, 101);
	execute_agi_command (0x61, p);
	TEST ("get.room.v", (test_report ("object %d in room %d", 2,
		getvar(101)), getvar (101) == 42 ? TEST_OK :
		(test_report (" [Expected: %d]", 42), TEST_FAIL)));
}

