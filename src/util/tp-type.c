#include <stdio.h>

int
main(void)
{
	int c;

	while (242) {
		c = fgetc(stdin);
		if (c == EOF)
			break;
		if (c == 0x1a)
			break;

		c = c & 0x7f;

		switch (c) {
			case 0x02: continue;
			case 0x05: continue;
			case 0x11: continue;
			case 0x12: continue;
			case 0x19: continue;
			case 0x1e: continue;

			case 0x0f: c = '-'; break;
			case 0x1f: c = '-'; break;
			case '{':  c = 'ä'; break;
			case '|':  c = 'ö'; break;
			case '}':  c = 'ü'; break;
			case ']':  c = 'Ü'; break;
			case '~':  c = 'ß'; break;
		}
		fputc(c, stdout);
	}

	return 0;
}
