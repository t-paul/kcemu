#include <stdio.h>
#include <curses.h>

static long _offset = 0;

void
question(const char *str, char *buf, int len)
{
	move(LINES - 1, 0);
	clrtoeol();
	mvprintw(LINES - 1, 0, "%s: ", str);
	nocbreak();
	echo();
	getnstr(buf, len);
	cbreak();
	noecho();
}

void
go(void)
{
	char buf[41];

	question("go to offset (hex)", buf, 40);
	_offset = strtoul(buf, 0, 16);
}

void
Go(void)
{
	char buf[41];

	question("go to offset", buf, 40);
	_offset = strtoul(buf, 0, 0);
}

void
show(unsigned char *buf)
{
	int a, b, c;

	erase();
	for (a = 0;a < LINES;a++) {
		mvprintw(a, 0, "%08x: ", _offset + a);
		c = buf[a] / 4;
		mvprintw(a, c + 12, "*");
		mvprintw(a, 80, "%3d | %10d", buf[a], _offset + a);
	}
	refresh();
}

void
disp(FILE *f)
{
	int key;
	unsigned char buf[1024];
	
	while (242) {
		fseek(f, _offset, SEEK_SET);
		fread(buf, 1, 1024, f);
		show(buf);
		key = getch();
		switch (key) {
		case 'q':
		case 'Q':
			return;
		case 'g':
			go();
			break;
		case 'G':
			Go();
			break;
		case KEY_UP:
			_offset -= 1;
			break;
		case KEY_DOWN:
			_offset += 1;
			break;
		case KEY_NPAGE:
			_offset += (LINES - 2);
			break;
		case KEY_PPAGE:
			_offset -= (LINES - 2);
			break;
		case '+':
			_offset += 1000;
			break;
		case '-':
			_offset -= 1000;
			break;
		}
	}
}

int
main(int argc, char **argv)
{
	FILE *f;

	if (argc < 2) {
		printf("usage: %s filename [offset]\n", argv[0]);
		exit(0);
	}

	if ((f = fopen(argv[1], "rb")) == 0) {
		perror("can't open file");
		exit(1);
	}

	if (argc >= 3) {
		_offset = atol(argv[2]);
	}

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	disp(f);

	fclose(f);
	endwin();

	return 0;
}
