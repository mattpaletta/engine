# Fetch FreeType
option(FREETYPE_VERSION "Freetype Version" VER-2-10-0)
fetch_extern(freetype2 https://github.com/aseprite/freetype2 ${FREETYPE_VERSION})

