#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kc/config.h"
#include "sys/sysdep.h"

char *
sys_gethome(void) {
    const char *ptr;
    struct passwd *passwd;
    
    ptr = getenv("HOME");
    if (ptr != NULL)
        return strdup(ptr);
    
    passwd = getpwuid(getuid());
    if (passwd != NULL)
        return strdup(passwd->pw_dir);
    
    return NULL;
}
