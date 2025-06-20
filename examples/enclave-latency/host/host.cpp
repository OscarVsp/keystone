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

static inline int64_t calctime(struct timespec t)
{
	int64_t time;
	time = USEC_PER_SEC * t.tv_sec;
	time += ((int) t.tv_nsec) / 1000;
	return time;
}

static inline void tsnorm(struct timespec *ts)
{
	while (ts->tv_nsec >= NSEC_PER_SEC) {
		ts->tv_nsec -= NSEC_PER_SEC;
		ts->tv_sec++;
	}
}

static inline int tsgreater(struct timespec *a, struct timespec *b)
{
	return ((a->tv_sec > b->tv_sec) ||
		(a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec));
}

static struct timespec enc_time = {0};

int
receive_enc_time(struct timespec* new_enc_time);
void
receive_enc_time_wrapper(void* buffer);
#define OCALL_ENC_TIME 1

int
receive_enc_time(struct timespec* new_enc_time) {
  enc_time = *new_enc_time;
  return 1;
}

int
main(int argc, char** argv) {
  
  Params params;

  params.setFreeMemSize(256 * 1024);
  params.setUntrustedSize(256 * 1024);

  int ret;
  int count;
  double avg_start, avg_end = 0.0;
  struct timespec start, end;

  printf("Starting enclave runtime test\n");


  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = 100 * 1000000L;  // 500 ms in nanoseconds

  for (count=0; count < N_ITER; count ++){
    
    Enclave enclave;
    int64_t start_enc, enc_end;

    ret = clock_gettime(CLOCK_MONOTONIC, &start);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}
    
    
    enclave.init(argv[1], argv[2], argv[3], params);
    enclave.registerOcallDispatch(incoming_call_dispatch);
    register_call(OCALL_ENC_TIME, receive_enc_time_wrapper);
    edge_call_init_internals(
        (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());
    enclave.run();

    ret = clock_gettime(CLOCK_MONOTONIC, &end);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}

    start_enc = calcdiff(start, enc_time);
    enc_end  = calcdiff(enc_time, end);

    avg_start += (double)start_enc / N_ITER;

    avg_end += (double)enc_end / N_ITER;

    

    if (count % 10 == 0) {
      printf("[%d/%d]\n", count, N_ITER);
    }

  }

  printf("Avg (%d interation):\n\tstart enclave latency: %fus\n\tend enclave latency: %fus\n", N_ITER, avg_start, avg_end);

  printf("Test finished\n");

  return 0;
}



void
receive_enc_time_wrapper(void* buffer) {
  /* Parse and validate the incoming call data */
  struct edge_call* edge_call = (struct edge_call*)buffer;
  uintptr_t call_args;
  int ret_val;
  size_t arg_len;
  if (edge_call_args_ptr(edge_call, &call_args, &arg_len) != 0) {
    edge_call->return_data.call_status = CALL_STATUS_BAD_OFFSET;
    return;
  }

  /* Pass the arguments from the eapp to the exported ocall function */
  ret_val = receive_enc_time((struct timespec*)call_args);

  /* Setup return data from the ocall function */
  uintptr_t data_section = edge_call_data_ptr();
  memcpy((void*)data_section, &ret_val, sizeof(int));
  if (edge_call_setup_ret(
          edge_call, (void*)data_section, sizeof(int))) {
    edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
  } else {
    edge_call->return_data.call_status = CALL_STATUS_OK;
  }

  /* This will now eventually return control to the enclave */
  return;
}