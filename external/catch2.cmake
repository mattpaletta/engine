find_package(Catch2 QUIET)

if (ENGINE_FORCE_DOWNLOAD_CATCH2 OR NOT ${Catch2_FOUND})
	fetch_extern(catch2 https://github.com/catchorg/Catch2 v2.12.1)
endif()
