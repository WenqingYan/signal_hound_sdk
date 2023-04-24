// Copyright (c).2021, Signal Hound, Inc.
// For licensing information, please see the API license in the software_licenses folder

#ifndef SHC_API_H
#define SHC_API_H

#if defined(_WIN32) // Windows
    #ifdef SHC_EXPORTS
        #define SHC_API __declspec(dllexport)
    #else
        #define SHC_API
    #endif
#else // Linux
    #include <stdint.h>
    #define SHC_API __attribute__((visibility("default")))
#endif

#define SHC_MAX_THREADS (8)
#define SHC_MIN_CHANNEL_COUNT (2)
#define SHC_MAX_CHANNEL_COUNT (2048)

#define SHC_OUTPUT_FORMAT_CONTIGUOUS (0)
#define SHC_OUTPUT_FORMAT_NON_CONTIGUOUS (1)

// Error codes
#define SHC_ERR_NO_ERR (0)
#define SHC_ERR_INVALID_HANDLE (-1)
#define SHC_ERR_NULL_PTR (-2)
#define SHC_ERR_INVALID_PARAMETER (-3)
#define SHC_ERR_QUEUE_ERR (-4)
#define SHC_ERR_INVALID_CONFIGURATION (-5)

#ifdef __cplusplus
extern "C" {
#endif

// Return: 1 if the CPU is capable of running the channelizer, 0 if not. The CPU requires
//   AVX instrinsic support. This includes most Intel Core CPUs from 3rd generation (3000 series)
//   and later.
SHC_API int shcIsCPUCapable();

// Convenience function for generating FIR filter coefficients.
// For a given channel count M and filter length K (filter size at decimated rate),
//   generate a filter with M*K taps.
SHC_API int shcGetFilterTaps(float *filter, int filterLen, double cutoff);

// Create a new channelizer
// M: Number of channels
// filter: real valued FIR filter array. This filter should be at the full sample rate.
//   For example, if you want the equivalent of a 16 tap filter over each channel,
//   this filter should be 16*M taps in length.
// filterLen: Length of filter array.
// N: Number of samples per channel to process on each API call. The number of input samples
//   to the channelizer will be M*N
// threads: The number of threads to use. If one thread is used, all processing will occur
//   in the calling thread. If 2 or more threads are specified, the calling thread will
//   wait for these threads to finish their processing.
// Return: Integer handle to the channelizer. Will be greater than 0. Returns negative
//   number if failed.
SHC_API int shcCreate(int M, const float *filter, int filterLen, int N, int threads);

// Free all resources associated with a channelizer
SHC_API int shcDestroy(int handle);

// Specify the output memory layout for data returned from the channelizer.
// format: Must be SHC_OUTPUT_FORMAT_CONTIGUOUS or SHC_OUTPUT_FORMAT_NON_CONTIGUOUS
SHC_API int shcSetOutputFormat(int handle, int format);

// Retrieve the expected input length for a channelizer
// Return: M*N for the specified channelizer
SHC_API int shcGetInputLength(int handle);

// Interface for single threaded operation.
// Processes input immediately into output.
// input: Array of M*N interleaved complex 32-bit floating point samples
// inputLen: Must equal M*N
// output: Output will depend on what format was selected with the shcSetOutputFormat
//   function. Refer to the examples to see how to configure each.
SHC_API int shcProcess(int handle, const float *input, int inputLen, void *output);

// Interface for multi-threaded channelizer. Each call to start queues up M*N samples. Can
//   queue up to 'ThreadCount' number inputs. Calling finish finished one item in queue. Queue
//   is FIFO.
// input: Array of M*N interleaved complex 32-bit floating point samples
// inputLen: Must equal M*N
// output: Output will depend on what format was selected with the shcSetOutputFormat
//   function. Refer to the examples to see how to configure each.
SHC_API int shcStart(int handle, const float *input, int inputLen);
SHC_API int shcFinish(int handle, void *output);

// Return: The number of M*N inputs in the queue, or put another way, the number of times
//   start has been called without an accompanying finish. Can return error code.
SHC_API int shcGetQueueSize(int handle);

#ifdef __cplusplus
} // Extern "C"
#endif

#endif // SHC_API_H
