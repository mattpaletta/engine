# Fetch OpenAL
# Requires SNDFile
set(BUILD_PROGRAMS OFF)
set(BUILD_EXAMPLES OFF)
set(ENABLE_CPACK OFF)
set(BUILD_TESTING OFF)
set(BUILD_REGTEST OFF)
set(ENABLE_PACKAGE_CONFIG OFF)
fetch_extern(sndfile https://github.com/erikd/libsndfile 06ebde50e362966184790c1b53512438a4385d47)

set(ALSOFT_EXAMPLES OFF)
set(ALSOFT_UTILS ON)
set(ALSOFT_INSTALL OFF)
set(ALSOFT_BUILD_ROUTER OFF)

fetch_extern(openal https://github.com/kcat/openal-soft openal-soft-1.20.1)
get_property(openal_SOURCE_DIR GLOBAL PROPERTY openal_SOURCE_DIR)
set(OpenAl_INCLUDE_DIRECTORIES ${openal_SOURCE_DIR}/include)
# SndFile::sndfile
set(OpenAl_DEPS OpenAL sndfile)
