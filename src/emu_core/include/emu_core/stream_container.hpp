#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace emu {

struct StreamContainer {
    std::istream *OpenInput(const std::string &file, bool is_binary);
    std::ostream *OpenOutput(const std::string &file, bool is_binary);

    std::istream *OpenTextInput(const std::string &file) { return OpenInput(file, false); }
    std::istream *OpenBinaryInput(const std::string &file) { return OpenInput(file, true); }
    std::ostream *OpenTextOutput(const std::string &file) { return OpenOutput(file, false); }
    std::ostream *OpenBinaryOutput(const std::string &file) { return OpenOutput(file, true); }

protected:
    std::vector<std::shared_ptr<std::istream>> input_streams;
    std::vector<std::shared_ptr<std::ostream>> output_streams;
};

} // namespace emu
