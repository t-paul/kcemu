#include <stdio.h>

int
main(void)
{
	int c;
	long idx;

	fputs("\xc3KC-TAPE by AF. ", stdout);
	idx = 0;
	while (242) {
		c = fgetc(stdin);
		idx++;
		if (c == EOF)
			break;
		if ((idx % 130) == 0)
			continue;
		fputc(c, stdout);
	}

	return 0;
}
