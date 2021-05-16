#pragma once

#include <emu_core/program.hpp>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace emu::assembler {

std::unique_ptr<Program> CompileString(const std::string &text);
std::unique_ptr<Program> CompileStream(std::istream &input);
std::unique_ptr<Program> CompileFile(const std::string &file_name);

} // namespace emu::assembler
