#include <stdio.h>

void
dump_sector(int c, int h, int s)
{
	int a;

	putchar(c); // acyl
	putchar(h); // asid
	putchar(c); // lcyl
	putchar(h); // lsid
	putchar(s); // lsec
	putchar(3); // llen
	putchar(0); // count low
	putchar(4); // count high

	for (a = 0;a < 1024;a++)
		putchar(0xe5);
}

int
main(void)
{
	int c, h, s;

	for (c = 0;c < 80;c++)
		for (h = 0;h < 2;h++)
			for (s = 1;s < 6;s++)
				dump_sector(c, h, s);

	return 0;
}
