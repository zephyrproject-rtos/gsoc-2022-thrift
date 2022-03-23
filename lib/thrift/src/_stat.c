#include <errno.h>
#include <sys/stat.h>

#include <zephyr.h>

int stat(const char *restrict path, struct stat *restrict buf)
{
    ARG_UNUSED(path);
    ARG_UNUSED(buf);

    errno = ENOTSUP;

    return -1;
}
