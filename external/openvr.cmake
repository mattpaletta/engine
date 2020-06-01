# Fetch OpenVR
set(OPENVR_VERSION v1.11.11)
set(BUILD_SHARED ON)
option(USE_LIBCXX "use libcxx for openvr" OFF)
fetch_extern(openvr https://github.com/ValveSoftware/openvr ${OPENVR_VERSION})
