#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/WinUtil.h>

static Window root;

void
send(Display *disp, const char *cmd, int len)
{
  Atom atom, type_ret;
  int a, b, format_ret;
  unsigned char *prop_ret;
  unsigned int nchildren_ret;
  unsigned long nitems_ret, bytes_after_ret;
  Window w, root, root_ret, parent_ret, *children_ret;

  atom = XInternAtom(disp, "_KCEMU_REMOTE_COMMAND", False);
  root = DefaultRootWindow(disp);
  
  if (!XQueryTree(disp, root,
                  &root_ret, &parent_ret,
                  &children_ret, &nchildren_ret))
    return;

  for (a = 0;a < nchildren_ret;a++)
    {
      w = XmuClientWindow(disp, children_ret[a]);
      XGetWindowProperty(disp, w,
                         atom, 0, (65536 / sizeof(long)), False,
                         XA_STRING, &type_ret, &format_ret,
                         &nitems_ret, &bytes_after_ret,
                         &prop_ret);
      if (type_ret == None)
        continue;
      printf("property found! on window %x -> %s\n", w, prop_ret);
      XChangeProperty(disp, w,
                      atom, XA_STRING, 8, PropModeReplace,
                      cmd, len);
    }
}

const char *
pack_args(int argc, char **argv, int *len_ret)
{
	int a, len;
	char *args, *ptr;
	
	printf("argc = %d\n", argc);

	len = 0;
	for (a = 1;a < argc;a++) {
		printf("argv[%d] = %s\n", a, argv[a]);
		len += strlen(argv[a]) + 1;
	}
	
	if (!(args = (char *)malloc(len)))
		return 0;

	ptr = args;
	for (a = 1;a < argc;a++) {
		strcpy(ptr, argv[a]);
		ptr += strlen(argv[a]) + 1;
	}
	
	*len_ret = len;
	return args;
}

int
main(int argc, char **argv)
{
  Display *disp;
  int len;
  const char *args;
  
  if (argc < 2)
	  return 1;

  disp = XOpenDisplay("");
  if (disp == 0)
    {
      printf("can't open display\n");
      exit(1);
    }
  
  args = pack_args(argc, argv, &len);
  if (!args)
	  return 1;

  send(disp, args, len);
  return 0;
}
