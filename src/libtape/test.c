#include <stdio.h>

int
main(void)
{
	int a;
	FILE *f;

	f = fopen("testfile.1", "wb");
	if (f) {
		for (a = 0;a < 512;a++) {
			fputc(a & 0xff, f);
		}
		fclose(f);
	}
	f = fopen("testfile.2", "wb");
	if (f) {
		for (a = 0;a < 512;a++) {
			fputc(((a >> 8) & 0xff) * 64 + 64, f);
		}
		fclose(f);
	}
	return 0;
}
