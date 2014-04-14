#include <sys/wait.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
int __pipe(__pipedes)
int __pipedes[2];
{
    if (__pipedes == NULL)
    {
        _set_errno(EINVAL);
        return -1;
    }

    return _pipe(__pipedes, 0, _O_BINARY | _O_NOINHERIT);
}