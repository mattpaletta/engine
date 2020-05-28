#pragma once

#if __has_include(<filesystem>)
#include <filesystem>
namespace constants {
	namespace fs = std::filesystem;
}
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace constants {
	namespace fs = std::experimental::filesystem;
}
#endif