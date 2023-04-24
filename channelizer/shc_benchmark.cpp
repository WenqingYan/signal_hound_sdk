#include "shc_benchmark.h"
#include "shc_api.h"

#include <cassert>
#include <chrono>
#include <complex>
#include <cstdint>
#include <vector>

static uint64_t GetCurrentMS()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
}

typedef std::complex<float> Cplx32f;

double shcBenchmark(int M, int K, double seconds, int threads, int inputSizeBytes, int outputFormat)
{
    int fullFilterLen = M * K;
    // How many bytes per channel should we process per iter, rounded up
    int bytesPerChannel = (int)ceil((double)inputSizeBytes / (double)M);
    // Account for 8 bytes per sample (complex)
    int N = bytesPerChannel /= 8;
    if(N < 1) {
        N = 1;
    }

    int inputSamples = M * N;

    // Generate filter
    double cutoff = 0.8 * (0.5 / M);
    std::vector<float> taps(fullFilterLen);
    shcGetFilterTaps(taps.data(), fullFilterLen, cutoff);

    // Create channelizer
    int handle = shcCreate(M, taps.data(), fullFilterLen, N, threads);
    if(handle <= 0) {
        return 0.0;
    }

    shcSetOutputFormat(handle, outputFormat);

    // Allocate input
    std::vector<Cplx32f> input(inputSamples);
    // CW input
    for(Cplx32f &c : input) {
        c.real(1.0);
        c.imag(0.0);
    }

    // Allocate output based on format type. Look at the examples for better ways of
    //   doing this based on format type.
    std::vector<uint8_t> output;
    std::vector<Cplx32f> nonContiguousArrays;

    if(outputFormat == SHC_OUTPUT_FORMAT_CONTIGUOUS) {
        output.resize(M*N*8);
    } else {
        output.resize(M*8);
        nonContiguousArrays.resize(M*N);
        Cplx32f **ptr = (Cplx32f**)output.data();
        for(int i = 0; i < M; i++) {
            ptr[i] = &nonContiguousArrays[i*N];
        }
    }

    // Determine how many iterations we need to do to target provided duration
    // Estimate that each thread can process ~100M samples per seoncd
    int64_t sps = (int64_t)100e6;
    int iters = (int)(seconds * (sps * threads) / inputSamples);

    uint64_t startTime = GetCurrentMS();

    if(threads == 1) {
        // Handle single threaded separately
        for(int iter = 0; iter < iters; iter++) {
            shcProcess(handle, (float*)input.data(), inputSamples, output.data());
        }
    } else {
        // Multi-threaded, keep maximum amount of data queued
        for(int i = 0; i < threads; i++) {
            shcStart(handle, (float*)input.data(), inputSamples);
        }

        for(int i = 0; i < iters-threads; i++) {
            shcFinish(handle, output.data());
            shcStart(handle, (float*)input.data(), inputSamples);
        }

        for(int i = 0; i < threads; i++) {
            shcFinish(handle, output.data());
        }
    }

    uint64_t elapsed = GetCurrentMS() - startTime;
    double samplesPerSecond = (double)(inputSamples * iters) / ((double)elapsed / 1000.0);
    return samplesPerSecond / 1.0e6;
}
