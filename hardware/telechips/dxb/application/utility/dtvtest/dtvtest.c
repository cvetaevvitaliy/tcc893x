/****************************************************************************
 *   FileName    : dumpmain.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>

int standard = 0;

void exit_test(int count)
{
	int i = 0;
	while (count > i++)
	{
		printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>> Start : Try %d\n\n", i);
		switch (standard)
		{
			case 0:
				system("am start -a android.intent.action.MAIN -n com.telechips.android.dvrdvbt/.DVBTPlayerActivity");
				break;
			case 1:
				system("am start -a android.intent.action.MAIN -n com.telechips.android.isdbt/.ISDBTPlayerActivity");
				break;
			default:
				return;
		}

		usleep(8000000);
		printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>> input keyevent KEYCODE_BACK\n\n");
		system("input keyevent 4");
		usleep(2000000);
		printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>> input keyevent KEYCODE_DPAD_RIGHT\n\n");
		system("input keyevent 22");
		usleep(2000000);
		printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>> input keyevent KEYCODE_DPAD_CENTER\n\n");
		system("input keyevent 66");
		usleep(8000000);
	}
}

void channel_test(int count)
{
	int i = 0;
	while (count > i++)
	{
		printf("\n\n >>>>>>>>>>>>>>>>>>>>>>>>>> input keyevent KEYCODE_PAGE_DOWN\n\n");
		system("input keyevent 93");
		usleep(10000000);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		printf(	"Usage : dtvtest standard test count\n"
				"        standard - dvb: 0, isdb: 1\n"
				"        test - channel change: 0, exit: 1\n"
				"        count - count\n");
	}

	sscanf(argv[1], "%d", &standard);

	int test = 0;
	sscanf(argv[2], "%d", &test);

	int count = 0;
	sscanf(argv[3], "%d", &count);

	if (test == 0)
		exit_test(count);
	else if (test == 1)
		channel_test(count);

    return 0;
}
