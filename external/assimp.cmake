set(ASSIMP_VERSION v5.0.0)
# set(BUILD_SHARED_LIBS ON)
option(ASSIMP_BUILD_ASSIMP_TOOLS "Assimp Tools" OFF)
option(ASSIMP_BUILD_BUILD_SAMPLES "Assimp samples" OFF)
option(ASSIMP_BUILD_TESTS "Assimp tests" OFF)
option(ASSIMP_INSTALL "Assimp install" ON)

fetch_extern(assimp https://github.com/assimp/assimp.git ${ASSIMP_VERSION})
