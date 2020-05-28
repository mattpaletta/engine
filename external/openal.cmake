# Fetch OpenAL

if (APPLE)
	set(BUILD_FRAMEWORK ON)
endif()

# Need latest patch for MacOS
set(OGG_VERSION "0bbcba4e7cf32324170470569c4527ffd0002870")
fetch_extern(ogg https://github.com/xiph/ogg ${OGG_VERSION})
get_property(ogg_SOURCE_DIR GLOBAL PROPERTY ogg_SOURCE_DIR)
set(OGG_INCLUDE_DIRS ${ogg_SOURCE_DIR}/include)
set(OGG_LIBRARIES ogg)
set(OGG_LIBRARY ogg)
set(OGG_INCLUDE_DIR ${ogg_SOURCE_DIR}/include)

if (APPLE)
	set(BUILD_FRAMEWORK OFF)
endif()

# Optional Vorbis Support
set(VORBIS_VERSION "v1.3.6")
#set(VORBIS_VERSION 0a4beb1d04f802c48016b11fb939690e24173168)
fetch_extern(vorbis https://github.com/xiph/vorbis.git ${VORBIS_VERSION})
get_property(vorbis_SOURCE_DIR GLOBAL PROPERTY vorbis_SOURCE_DIR)
set(Vorbis_Vorbis_INCLUDE_DIR ${vorbis_SOURCE_DIR}/include)
set(Vorbis_Vorbis_LIBRARY vorbis)

# Requires SNDFile
set(ENABLE_EXTERNAL_LIBS ON)
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

set(OPENAL_VERSION openal-soft-1.20.1)
fetch_extern(openal https://github.com/kcat/openal-soft ${OPENAL_VERSION})
get_property(openal_SOURCE_DIR GLOBAL PROPERTY openal_SOURCE_DIR)
set(OpenAl_INCLUDE_DIRECTORIES "${Vorbis_Vorbis_INCLUDE_DIR}")
# SndFile::sndfile
set(OpenAl_DEPS OpenAL sndfile ogg vorbis vorbisfile vorbisenc)
