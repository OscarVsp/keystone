#include "eapp_utils.h"
#include "edge_call.h"
#include <syscall.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>

#define OCALL_ENC_TIME 1

int ocall_enc_time();

int main()
{

  struct timespec now;
  int ret;
  ocall_enc_time();
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }
  ocall_enc_time(&now);
  return 0;
}

int ocall_enc_time(){
  ocall(OCALL_ENC_TIME, NULL, 0, NULL, 0);
  return 1;
}