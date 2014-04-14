#ifndef IOCTL_H
#define IOCTL_H
#include <windows.h>
#include <errno.h>
inline int
ioctl (int fd, unsigned long int request, ...)
{
  void *arg;
  va_list ap;
  int result;

  va_start (ap, request);
  arg = va_arg (ap, void *);

  switch (request)
    {
    default:
      _set_errno (ENOSYS);
      result = -1;
      break;
    }
  va_end (ap);
  return result;
}
#endif