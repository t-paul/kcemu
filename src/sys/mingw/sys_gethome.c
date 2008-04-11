#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kc/config.h"
#include "sys/sysdep.h"

static char *
_sys_gethome(void) {
    char *buf;
    const char *ptr, *ptr2;

    ptr = getenv("APPDATA");
    if (ptr != NULL) {
        buf = malloc(strlen(ptr) + 7);
	strcpy(buf, ptr);
	strcat(buf, "\\KCemu");
        return strdup(ptr);
    }

    ptr = getenv("USERPROFILE");
    if (ptr != NULL)
        return strdup(ptr);

    ptr = getenv("HOMEDRIVE");
    if (ptr != NULL) {
        ptr2 = getenv("HOMEPATH");
	if (ptr2 != NULL) {
	    buf = malloc(strlen(ptr) + strlen(ptr2) + 1);
	    strcpy(buf, ptr);
	    strcat(buf, ptr2);
            return buf;
	}
    }
    
    ptr = getenv("HOME");
    if (ptr != NULL)
        return strdup(ptr);
    
    return NULL;
}

char *
sys_gethome(void) {
    int a;
    char *ptr;

    ptr = _sys_gethome();
    if (ptr == NULL)
        return NULL;

    for (a = 0;ptr[a] != 0;a++)
        if (ptr[a] == '\\')
            ptr[a] = '/';

    return ptr;
}
