//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include "edge/edge_call.h"
#include "host/keystone.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>

using namespace Keystone;

#define N_ITER 100


#define MSEC_PER_SEC		1000
#define USEC_PER_SEC		1000000
#define NSEC_PER_SEC		1000000000
#define USEC_TO_NSEC(u)		((u) * 1000)
#define USEC_TO_SEC(u)		((u) / USEC_PER_SEC)
#define NSEC_TO_USEC(n)		((n) / 1000)
#define SEC_TO_NSEC(s)		((s) * NSEC_PER_SEC)
#define SEC_TO_USEC(s)		((s) * USEC_PER_SEC)

static inline int64_t calcdiff(struct timespec t1, struct timespec t2)
{
	int64_t diff = USEC_PER_SEC * (long long)((int) t1.tv_sec - (int) t2.tv_sec);
	diff += ((int) t1.tv_nsec - (int) t2.tv_nsec) / 1000;
	return diff;
}

static inline int64_t calcdiff_ns(struct timespec t1, struct timespec t2)
{
	int64_t diff;
	diff = NSEC_PER_SEC * (int64_t)((int) t1.tv_sec - (int) t2.tv_sec);
	diff += ((int) t1.tv_nsec - (int) t2.tv_nsec);
	return diff;
}

int
main(int argc, char** argv) {
  
  Params params;

  params.setFreeMemSize(256 * 1024);
  params.setUntrustedSize(256 * 1024);

  int ret;
  int count;
  double avg = 0.0;
  struct timespec previous, now;

  ret = clock_gettime(CLOCK_MONOTONIC, &now);
  if (ret != 0) {
    printf("Error getting time. error code: %d\n", errno);
    return -1;
  }

  printf("Starting enclave runtime test\n");


  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = 100 * 1000000L;  // 500 ms in nanoseconds

  for (count=0; count < N_ITER; count ++){
    
    Enclave enclave;
    int64_t lat;

    ret = clock_gettime(CLOCK_MONOTONIC, &previous);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}
    
    
    enclave.init(argv[1], argv[2], argv[3], params);
    enclave.registerOcallDispatch(incoming_call_dispatch);
    edge_call_init_internals(
        (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());
    enclave.run();

    ret = clock_gettime(CLOCK_MONOTONIC, &now);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}

    lat = calcdiff(now, previous);

    avg += (double)lat/ N_ITER;

    

    if (count % 10 == 0) {
      printf("[%d/%d]\n", count, N_ITER);
    }

    usleep(10); //Remove sleep
  }

  printf("Avg (%d interation): %fus\n", N_ITER, avg);

  printf("Test finished\n");

  return 0;
}