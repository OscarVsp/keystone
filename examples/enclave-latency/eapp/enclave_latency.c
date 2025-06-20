#include "eapp_utils.h"
#include "edge_call.h"
#include <syscall.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#define MSEC_PER_SEC		1000
#define USEC_PER_SEC		1000000
#define NSEC_PER_SEC		1000000000
#define USEC_TO_NSEC(u)		((u) * 1000)
#define USEC_TO_SEC(u)		((u) / USEC_PER_SEC)
#define NSEC_TO_USEC(n)		((n) / 1000)
#define SEC_TO_NSEC(s)		((s) * NSEC_PER_SEC)
#define SEC_TO_USEC(s)		((s) * USEC_PER_SEC)

#define OCALL_ENC_TIME 1

int ocall_enc_time();


static inline int64_t calcdiff(struct timespec t1, struct timespec t2)
{
	int64_t diff = USEC_PER_SEC * (long long)((int) t1.tv_sec - (int) t2.tv_sec);
	diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
	return diff;
}

int main()
{

  struct timespec before, after;
  double delay;
  int ret;
  ret = clock_gettime(CLOCK_REALTIME , &before);
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }
  //printf("in enclave start: %ld.%09ld\n", before.tv_sec, before.tv_nsec);

  ocall_enc_time();

  ret = clock_gettime(CLOCK_REALTIME , &after);
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }
  //printf("in enclave end: %ld.%09ld\n", after.tv_sec, after.tv_nsec);

  delay = (double)calcdiff(after, before);

  printf("\tOCCAL delay: %fus\n", delay);

  return 1;
}

int ocall_enc_time(){
  ocall(OCALL_ENC_TIME, NULL, 0, NULL, 0);
  return 1;
}