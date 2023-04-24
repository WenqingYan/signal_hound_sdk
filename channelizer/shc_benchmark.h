#ifndef SHC_BENCHMARK_H
#define SHC_BENCHMARK_H

// M = number of channels
// K = filter length at downsampled rate, full filter size = M * K
// seconds = estimated length of test, estimates 100MS/s per thread throughput.
// threads = number of threads to use, must be between [1,8]
// inputSizeBytes = target input size in bytes
// outputFormat = Set to either SHC_OUTPUT_FORMAT_NON_CONTIGUOUS or SHC_OUTPUT_FORMAT_CONTIGUOUS
// Returns samples per second in MS/s
double shcBenchmark(int M, int K, double seconds, int threads, int inputSizeBytes, int outputFormat);

#endif // SHC_BENCHMARK_H
