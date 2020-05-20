# Fetch JSON library
option(JSON_VERSION "JSON library version" v3.7.3)
fetch_extern(json https://github.com/nlohmann/json ${JSON_VERSION})
