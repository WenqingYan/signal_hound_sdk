#ifndef SHC_EXAMPLES_H
#define SHC_EXAMPLES_H

// The first two examples use the single threaded interface.
// These two examples differ only in how the output data is stored.
// They illustrate the differences in output format.
void shcExampleNonContiguousSingleThreaded();
void shcExampleContiguousSingleThreaded();

// This example illustrates using the multi-threaded interface.
// Contiguous memory output is used. For an example of using non-contiguous memory, see
//   the single threaded examples.
void shcExampleMultiThreaded();

#endif // SHC_EXAMPLES_H
