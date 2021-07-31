#pragma once

#include <filesystem>
#include <fstream>
#include <string>

namespace emu {

template <typename container>
inline void save_file(const std::string &p, const container &str) {
    std::ofstream file;
    file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    file.open(p, std::ios_base::binary);
    file.write(reinterpret_cast<const char *>(&str[0]), str.size());
}

template <typename container>
inline container load_file(const std::string &p) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(p, std::ios_base::binary);
    std::size_t sz = static_cast<std::size_t>(std::filesystem::file_size(p));
    container r;
    r.resize(sz, 0);
    file.read(reinterpret_cast<char *>(&r[0]), static_cast<std::streamsize>(sz));
    return r;
}

} // namespace emu
