#include <time.h>
#include <stdio.h>
#include <errno.h>

int main()
{

  struct timespec now;
  int ret;
  ret = clock_gettime(CLOCK_MONOTONIC, &now);
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }
  printf("%ld,%ld\n", now.tv_sec, now.tv_nsec);
  return 0;
}
