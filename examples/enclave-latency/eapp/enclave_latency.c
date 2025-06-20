#include "eapp_utils.h"
#include "edge_call.h"
#include <syscall.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#define OCALL_ENC_TIME 1

int ocall_enc_time(struct timespec* enc_time);

int main()
{

  struct timespec now;
  int ret;
  ret = clock_gettime(CLOCK_MONOTONIC, &now);
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }
  ocall_enc_time(&now);
  return 0;
}

int ocall_enc_time(struct timespec* enc_time){
  int retval;
  ocall(OCALL_ENC_TIME, enc_time, sizeof(enc_time), &retval ,sizeof(int));
  return retval;
}