#include <math.h>
#include <stdio.h>

#define N (256)

static FILE *_f;
static int _buf[N];
static int _i = 0;
int _bytes = 0;

void
read_next(int len)
{
	int a, c;

	for (a = 0;a < len;a++) {
		_bytes++;
		c = fgetc(_f);
		if (c == EOF)
			exit(2);
		_buf[_i] = c - 128;
		_i = (_i + 1) % N;
	}
	//printf(" -> %d\n", _i);
}

#define WAVELEN (64)
static char _wave_line[WAVELEN + 1] = "A";

void
draw_wave(int b, int flag, char x)
{
	int a, c;

	if (*_wave_line == 'A')
		memset(_wave_line, ' ', WAVELEN);

	if (b < -128)
		b = -128;
	if (b > 127)
		b = 127;

	c = (b + 128) / 4;
	for (a = 0;a < WAVELEN;a++) {
		if (flag) _wave_line[a] = '*';
		if (a == c) _wave_line[a] = x;
	}
}
void
show_wave()
{
	_wave_line[WAVELEN] = '\0';
	printf("%s", _wave_line);
	memset(_wave_line, ' ', WAVELEN);
}

#if 1
#define X(x)
#else
#define X(x) do { if (_bytes < 10000) break; x ; } while (0);
#endif

int
loop(void)
{
	int a, b, c, v1, v2, v3, x, flag, do_read;
	int o1, o2, o3;
	static int level = 2;

	do_read = 1;
	for (a = 0;a < N - 20;a++) {

		o1 = a;
		o2 = a + 4;
		o3 = a + 8;

		v1  = _buf[(_i + o1 + 1) % N] - _buf[(_i + o1    ) % N];
		v1 += _buf[(_i + o1 + 2) % N] - _buf[(_i + o1 + 1) % N];
		v1 += _buf[(_i + o1 + 3) % N] - _buf[(_i + o1 + 2) % N];
		v1 += _buf[(_i + o1 + 4) % N] - _buf[(_i + o1 + 3) % N];
		v1 /= 4;

		v2  = _buf[(_i + o2 + 1) % N] - _buf[(_i + o2    ) % N];
		v2 += _buf[(_i + o2 + 2) % N] - _buf[(_i + o2 + 1) % N];
		v2 += _buf[(_i + o2 + 3) % N] - _buf[(_i + o2 + 2) % N];
		v2 += _buf[(_i + o2 + 4) % N] - _buf[(_i + o2 + 3) % N];
		v2 /= 4;

		v3  = _buf[(_i + o3 + 1) % N] - _buf[(_i + o3    ) % N];
		v3 += _buf[(_i + o3 + 2) % N] - _buf[(_i + o3 + 1) % N];
		v3 += _buf[(_i + o3 + 3) % N] - _buf[(_i + o3 + 2) % N];
		v3 += _buf[(_i + o3 + 4) % N] - _buf[(_i + o3 + 3) % N];
		v3 /= 4;

		flag = 0;
		if (level < 0) {
			if ((v1 <= -1) && (v3 >= 1) && (v2 > -1) && (v2 < 1)) {
				flag = 1;
				level = -level;
			}
		} else {
			if ((v1 >= 1) && (v3 <= -1) && (v2 > -1) && (v2 < 1)) {
				flag = 1;
				level = -level;
			}
		}

		X(printf("%8d: ", _bytes - N + a));
		X(draw_wave(_buf[(_i + a) % N], flag, '.'));
		X(show_wave());
		X(printf("%4d, %4d, %4d\n", v1, v2, v3));
		if (flag) {
			new_counter(a + 1);
			handle_half_wave(0, a + 1);
 X(printf("XXX: %d / %d\n", a + 1, _bytes));
			read_next(a + 1);
			return 1;
		}
	}
	read_next(N - 20);

	return 1;
}

int
loop3(void)
{
	int a, b, c, v1, v2, v3, x, flag, do_read;
	int o1, o2, o3;
	static int level = 2;

	do_read = 1;
	for (a = 0;a < N - 20;a++) {

		o1 = a;
		o2 = a + 4;
		o3 = a + 8;

		v1  = _buf[(_i + o1 + 0) % N];
		v1 += _buf[(_i + o1 + 1) % N];
		v1 += _buf[(_i + o1 + 2) % N];
		v1 += _buf[(_i + o1 + 3) % N];
		v1 += _buf[(_i + o1 + 4) % N];
		v1 += _buf[(_i + o1 + 5) % N];
		v1 /= 6;

		v2  = _buf[(_i + o2 + 0) % N];
		v2 += _buf[(_i + o2 + 1) % N];
		v2 += _buf[(_i + o2 + 2) % N];
		v2 += _buf[(_i + o2 + 3) % N];
		v2 += _buf[(_i + o2 + 4) % N];
		v2 += _buf[(_i + o2 + 5) % N];
		v2 /= 6;

		v3  = _buf[(_i + o3 + 0) % N];
		v3 += _buf[(_i + o3 + 1) % N];
		v3 += _buf[(_i + o3 + 2) % N];
		v3 += _buf[(_i + o3 + 3) % N];
		v3 += _buf[(_i + o3 + 4) % N];
		v3 += _buf[(_i + o3 + 5) % N];
		v3 /= 6;

		flag = 0;
		if (level < 0) {
			if ((v1 <= v2) && (v3 < v2)) {
				flag = 1;
				level = -level;
			}
		} else {
			if ((v1 >= v2) && (v3 > v2)) {
				flag = 1;
				level = -level;
			}
		}

		X(printf("%8d: ", _bytes - N + a));
		X(draw_wave(_buf[(_i + a) % N], flag, '.'));
		X(show_wave());
		X(printf("%4d, %4d, %4d\n", v1, v2, v3));
		if (flag) {
			new_counter(a + 1);
			handle_half_wave(0, a + 1);
 X(printf("XXX: %d / %d\n", a + 1, _bytes));
			read_next(a + 1);
			return 1;
		}
	}
	read_next(N - 20);

	return 1;
}

#if 0
#define Y(x)
#else
#define Y(x) do { if (_bytes < 0) break; x ; } while (0);
#endif

// 350000

static double _b[N];

void
loop2_init(void)
{
	int a;

	for (a = 0;a < N;a++) {
		_b[a] = sin((30.0 * M_PI * a) / N);
		Y(printf("%8d: ", a));
		Y(draw_wave(_b[a] * 50, 0, '.'));
		Y(show_wave());
		Y(printf("%4d\n", a));
	}
}

int
loop2(void)
{
	int a, b, c;
	double sum;
	int min, max;

	for (a = 0;a < N - 40;a++) {
		sum = 0;
		min = 300;
		max = -300;
		for (b = 0;b < 18;b++) {
			c =  _buf[(_i + a + b) % N];
			sum += c * _b[b];
			if (min > c) min = c;
			if (max < c) max = c;
		}
		min = abs(min);
		max = abs(max);
		if (max < min) max = min;
		sum /= max;
		Y(printf("%8d: ", _bytes - N + a));
		Y(draw_wave(_buf[(_i + a) % N], 0, '.'));
		Y(draw_wave(sum / 40.0, 0, 'o'));
		Y(show_wave());
		Y(printf("%c ", abs(sum) > 10 ? '#' : ' '));
		Y(printf("%-3.1f\n", sum));
	}

	read_next(N - 40);
	
	return 1;
}

int
main(void)
{
	int a, c;

	//_f = fopen("/home/tp/projects/KCemu-WAV/out.wav", "rb");
	//if (_f == NULL)
	//	exit(1);

	//fseek(_f, 205368, SEEK_SET);
	//fseek(_f, 205000, SEEK_SET);
	//fseek(_f, 100000, SEEK_SET);
	_f = stdin;
	read_next(N);

	init(1);
	// loop2_init();
	while (loop3());

	return 0;
}
