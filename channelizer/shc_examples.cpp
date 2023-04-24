#include "shc_examples.h"
#include "shc_api.h"

#include <cassert>
#include <complex>
#include <cstdint>
#include <vector>

typedef std::complex<float> Cplx32f;

void shcExampleNonContiguousSingleThreaded()
{
    // Number of output channels
    int M = 4;
    // Filter size, at decimated channel rate
    int K = 16;
    // Filter size at input sample rate
    int fullFilterLen = M * K;
    // Process N samples per channel per submit
    int N = 8192;
    // Number of samples we will feed to the channelizer per call
    int inputSize = M * N;

    // Generate filter
    // Set bandwidth to 75% of the decimated channel bandwidth
    double cutoff = 0.75 * (0.5 / M);
    std::vector<float> taps(fullFilterLen);
    shcGetFilterTaps(taps.data(), fullFilterLen, cutoff);

    // Create channelizer
    int handle = shcCreate(M, taps.data(), fullFilterLen, N, 1);
    assert(handle > 0);

    // Allocate input
    std::vector<Cplx32f> input(inputSize);
    // Set CW input
    for(Cplx32f &c : input) {
        c.real(1.0);
        c.imag(0.0);
    }

    shcSetOutputFormat(handle, SHC_OUTPUT_FORMAT_NON_CONTIGUOUS);

    // Allocate outputs, create M, N length output arrays
    std::vector<Cplx32f*> output;
    output.resize(M);
    for(int m = 0; m < M; m++) {
        output[m] = new Cplx32f[N];
    }

    // Process the input
    int sts = shcProcess(handle, (float*)input.data(), inputSize, (float**)output.data());
    assert(sts == 0);

    // Do something with output data

    shcDestroy(handle);
}

void shcExampleContiguousSingleThreaded()
{
    // Number of output channels
    int M = 4;
    // Filter size, at decimated channel rate
    int K = 16;
    // Filter size at input sample rate
    int fullFilterLen = M * K;
    // Process N samples per channel per submit
    int N = 8192;
    // Number of samples we will feed to the channelizer per call
    int inputSize = M * N;

    // Generate filter
    // Set bandwidth to 75% of the decimated channel bandwidth
    double cutoff = 0.75 * (0.5 / M);
    std::vector<float> taps(fullFilterLen);
    shcGetFilterTaps(taps.data(), fullFilterLen, cutoff);

    // Create channelizer
    int handle = shcCreate(M, taps.data(), fullFilterLen, N, 1);
    assert(handle > 0);

    // Allocate input
    std::vector<Cplx32f> input(inputSize);
    // Set CW input
    for(Cplx32f &c : input) {
        c.real(1.0);
        c.imag(0.0);
    }

    shcSetOutputFormat(handle, SHC_OUTPUT_FORMAT_CONTIGUOUS);

    // Allocate outputs, create M, N length output arrays
    std::vector<Cplx32f> output(M*N);

    // Process the input
    int sts = shcProcess(handle, (float*)input.data(), inputSize, output.data());
    assert(sts == 0);

    // Do something with output data
    // You can retrieve any output channel by using the formula
    // Cplx32f *channelPointer = &output[channelNumber * N];
    // where channelNumber is the index of the channel between [0,M-1]
    // The channelPointer will point to the first sample in the returned data
    //   for that channel. There will be N complex output samples per channel.

    shcDestroy(handle);
}

void shcExampleMultiThreaded()
{
    // Number of threads the channelizer will use. Also the size of the processing queue.
    int threads = 4;
    // Number of output channels
    int M = 4;
    // Filter size, at decimated channel rate
    int K = 16;
    // Filter size at input sample rate
    int fullFilterLen = M * K;
    // Process N samples per channel per submit
    int N = 1024;
    // Number of samples we will feed to the channelizer per call
    int inputSize = M * N;

    // Generate filter
    // Set bandwidth to 75% of the decimated channel bandwidth
    double cutoff = 0.75 * (0.5 / M);
    std::vector<float> taps(fullFilterLen);
    shcGetFilterTaps(taps.data(), fullFilterLen, cutoff);

    // Create channelizer
    int handle = shcCreate(M, taps.data(), fullFilterLen, N, 1);
    assert(handle > 0);

    // Allocate input
    std::vector<Cplx32f> input(inputSize);
    // Set CW input
    for(Cplx32f &c : input) {
        c.real(1.0);
        c.imag(0.0);
    }

    shcSetOutputFormat(handle, SHC_OUTPUT_FORMAT_CONTIGUOUS);

    // Allocate outputs, create M, N length output arrays
    std::vector<Cplx32f> output(M*N);

    // In this example, we will be feeding the same input samples to the channelizer
    // In a real application, you would be feeding the next M*N samples to the channelizer
    //   on each call.

    // In this example, we want to process J, M*N input blocks, for a total number of samples
    //   of J*M*N. Ideally J is larger than the number of threads being used in the channelizer
    //   to take advantage the parallelism
    const int J = 100;
    for(int j = 0; j < J; j++) {
        // If we already have a queue of inputs being processed, we must finish one
        //   once we have reached 'thread' number of queued inputs.
        if(shcGetQueueSize(handle) == threads) {
            shcFinish(handle, output.data());
            // Process output data here
        }

        // Queue up the next input.
        // In a real application, this would be the next M*N samples in your I/Q data stream.
        shcStart(handle, (float*)input.data(), inputSize);
    }

    // At this point all input samples have been queued into the channelizer. There are still
    //   'thread' inputs still queued. We can retrieve them now
    while(shcGetQueueSize(handle) > 0) {
        shcFinish(handle, output.data());
        // Process output data here
    }

    // Do something with output data
    // You can retrieve any output channel by using the formula
    // Cplx32f *channelPointer = &output[channelNumber * N];
    // where channelNumber is the index of the channel between [0,M-1]
    // The channelPointer will point to the first sample in the returned data
    //   for that channel. There will be N complex output samples per channel.

    shcDestroy(handle);
}
