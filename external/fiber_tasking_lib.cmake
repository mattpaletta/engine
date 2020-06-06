# You can use the FTL_BUILD_TESTS variable to enable / disable test building
option( FTL_BUILD_TESTS "FTL Test" OFF)

# You can use the FTL_BUILD_BENCHMARKS variable to enable / disable benchamark building
option( FTL_BUILD_BENCHMARKS "FTL benchmarks" OFF)

set(FTL_VERSION 7a4a642e54f307499c8d2da84d4a93b977835406)

fetch_extern(fiber_tasking_lib https://github.com/RichieSams/FiberTaskingLib.git ${FTL_VERSION})
