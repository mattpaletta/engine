include(FetchContent)

function(fetch_extern name repo tag)
        FetchContent_Declare(
                ${name}
                GIT_REPOSITORY ${repo}
                GIT_TAG ${tag}
        )
        FetchContent_GetProperties(${name})
	if (NOT ${name}_POPULATED)
	        FetchContent_Populate(${name})
	endif()
	set(source_dir "${${name}_SOURCE_DIR}")
	set(binary_dir "${${name}_BINARY_DIR}")
	add_subdirectory(${source_dir} ${binary_dir} EXCLUDE_FROM_ALL)

        # Store source & binary dir as global variables
        set_property(GLOBAL PROPERTY ${name}_SOURCE_DIR ${source_dir})
        set_property(GLOBAL PROPERTY ${name}_BINARY_DIR ${binary_dir})
endfunction(fetch_extern)

# This is practically the same, except it deals with URLS
function(fetch_extern_url name url)
        FetchContent_Declare(
                ${name}
                URL ${url}
        )
        FetchContent_GetProperties(${name})
        if (NOT ${name}_POPULATED)
                FetchContent_Populate(${name})
        endif()
        set(source_dir ${${name}_SOURCE_DIR})
        set(binary_dir ${${name}_BINARY_DIR})

        # Store source & binary dir as global variables
        set_property(GLOBAL PROPERTY ${name}_SOURCE_DIR ${source_dir})
        set_property(GLOBAL PROPERTY ${name}_BINARY_DIR ${binary_dir})
endfunction(fetch_extern_url)
