#set(EIGEN_VERSION 3.3.7)
set(EIGEN_VERSION 4e7046063babd95e100ab202d52ca5776997c80c)
option(EIGEN_BUILD_PKGCONFIG ON)
option(EIGEN_SPLIT_LARGE_TESTS OFF)
set(BUILD_TESTING OFF)

fetch_extern(eigen https://gitlab.com/libeigen/eigen.git ${EIGEN_VERSION})
