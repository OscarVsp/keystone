//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include "edge/edge_call.h"
#include "host/keystone.h"
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

using namespace Keystone;

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
receive_enc_time();
void
receive_enc_time_wrapper(void* buffer);
#define OCALL_ENC_TIME 1


//FIXME: timespec nsec is not the one printed from the enclave, and stay const acros time
int
receive_enc_time() {
  int ret;
  ret = clock_gettime(CLOCK_REALTIME , &enc_time);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}
  return 1;
}

int
main(int argc, char** argv) {
  
  Params params;

  int iter = atoi(argv[4]);;

  params.setFreeMemSize(256 * 1024);
  params.setUntrustedSize(256 * 1024);

  int ret;
  int count;
  int64_t avg_start, avg_end = 0.0;
  struct timespec start, end;

  printf("Starting enclave runtime test\n");


  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = 100 * 1000000L;  // 500 ms in nanoseconds

  for (count=0; count < iter; count ++){
    
    Enclave enclave;
    int64_t start_enc, enc_end;

    ret = clock_gettime(CLOCK_REALTIME , &start);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}
    //printf("host start: %ld.%09ld\n", start.tv_sec, start.tv_nsec);

    printf("[%d/%d]\n", count, iter);
    
    enclave.init(argv[1], argv[2], argv[3], params);
    enclave.registerOcallDispatch(incoming_call_dispatch);
    register_call(OCALL_ENC_TIME, receive_enc_time_wrapper);
    edge_call_init_internals(
        (uintptr_t)enclave.getSharedBuffer(), enclave.getSharedBufferSize());
    enclave.run();

    ret = clock_gettime(CLOCK_REALTIME , &end);
		if (ret != 0) {
			printf("Error getting time. error code: %d\n", errno);
			return -1;
		}
    //printf("enclave received: %ld.%09ld\n", enc_time.tv_sec, enc_time.tv_nsec);
    //printf("end: %ld.%09ld\n", end.tv_sec, end.tv_nsec);

    start_enc = calcdiff(enc_time,start);
    enc_end  = calcdiff(end, start);

    printf("\tEnclave start duration: %" PRId64 "us\n", start_enc);
    printf("\tEnclave run duration: %" PRId64 "us\n", enc_end);

    avg_start += start_enc / iter;

    avg_end += enc_end / iter;


  }

  printf("-----------------------------------------\n");

  printf("Avg (%d interation):\n\tstart enclave latency: %" PRId64 "us\n\tend enclave latency: %" PRId64 "us\n\ttotal enclave latency: %" PRId64 "us\n", iter, avg_start, avg_end, avg_start+avg_end);

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
  ret_val = receive_enc_time();

  /* Setup return data from the ocall function */
  uintptr_t data_section = edge_call_data_ptr();
  memcpy((void*)data_section, nullptr, 0);
  if (edge_call_setup_ret(
          edge_call, (void*)data_section, 0)) {
    edge_call->return_data.call_status = CALL_STATUS_BAD_PTR;
  } else {
    edge_call->return_data.call_status = CALL_STATUS_OK;
  }

  /* This will now eventually return control to the enclave */
  return;
}