# Fetch OpenVR
set(OPENVR_VERSION v1.11.11)
set(BUILD_SHARED ON)
set(USE_LIBCXX OFF)
fetch_extern(openvr https://github.com/ValveSoftware/openvr ${OPENVR_VERSION})
