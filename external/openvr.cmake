# Fetch OpenVR
option(OPENVR_VERSION "OpenVR Version" v1.11.11)
set(BUILD_SHARED ON)
set(USE_LIBCXX OFF)
fetch_extern(openvr https://github.com/ValveSoftware/openvr ${OPENVR_VERSION})
