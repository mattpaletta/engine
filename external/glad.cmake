# Fetch GLAD
option(GLAD_VERSION "Glad Version" v0.1.33)
set(GLAD_PROFILE "core")
set(GLAD_API "gl=3.3")
set(GLAD_GENERATOR "c")
set(GLAD_REPRODUCIBLE ON)
set(GLAD_ALL_EXTENSIONS ON)
fetch_extern(glad https://github.com/Dav1dde/glad ${GLAD_VERSION})
