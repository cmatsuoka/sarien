#include <sys/types.h>
#include <sys/stat.h>
#include "orat.h"
#include "test.h"


#define PIC_SIZE 26880

static int compare_pic (char *p)
{
	FILE *raw;
	UINT8 buf[PIC_SIZE];

	if ((raw = fopen (p, "rb")) == NULL)
		return 0;

	fread (buf, PIC_SIZE, 1, raw);
	fclose (raw);
	
	return memcmp (buf, game.sbuf, PIC_SIZE) ? TEST_FAIL : TEST_OK; 
}


static int load_pic (char *p)
{
	FILE *res;
	struct stat st;
	char *x;
	int len;

	if ((res = fopen (p, "rb")) == NULL)
		return TEST_FAIL;

	fstat (fileno(res),&st);
	len = st.st_size;
	
	x = malloc (len);
	fread (x, 1, len, res);
	fclose (res);

	game.pictures[0].rdata = x;
	game.dir_pic[0].len = len;

	return TEST_OK;
}


static test_result pic (char *res, char *raw)
{
	int i;
	test_result result = TEST_OK;

	if (load_pic (res) == TEST_FAIL)
		return TEST_FAIL;

	test_report ("drawing and comparing 50x");

	for (i = 0; i < 50; i++) {
		decode_picture (0,1);
		if ((result = compare_pic (raw)) != TEST_OK) 
			break;
	}

	free (game.pictures[0].rdata);

	return result;
}


TEST_MODULE(test_picture)
{
	TEST("picture #1 (test pic)",
		pic("data/picture1.res", "data/picture1.raw"));
	TEST("picture #2 (op-recon)",
		pic("data/picture2.res", "data/picture2.raw"));
	TEST("picture #3 (ruby cast)",
		pic("data/picture3.res", "data/picture3.raw"));
}

