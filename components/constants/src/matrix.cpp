#include "matrix.hpp"

std::string to_string(const glm::mat4& mat) {
    std::string out;
    for (int i = 0; i < 4; ++i) {
        out += "[";
        for (int j = 0; j < 4; ++j) {
            out += std::to_string(mat[i][j]);
            out += " ";
        }
        out += "]\n";
    }
    return out;
}