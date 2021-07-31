#pragma once

#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <stdexcept>

namespace emu::program_options {

namespace po = boost::program_options;

using ConflictingOptionsVec = std::vector<std::pair<std::string, std::string>>;

// void OptionDependency(const variables_map &vm, const char *for_what, const char *required_option) {
//     if (vm.count(for_what) && !vm[for_what].defaulted())
//         if (vm.count(required_option) == 0 || vm[required_option].defaulted())
//             throw std::logic_error(string("Option '") + for_what + "' requires option '" + required_option + "'.");
// }

// void ConflictingOptions(const po::variables_map &vm, const ConflictingOptionsVec &cov) {
//     for (auto &[a, b] : cov) {
//         OptionDependency(vm, a.c_str(), b.c_str());
//     }
// }

void ConflictingOptions(const po::variables_map &vm, const char *opt1, const char *opt2) {
    if ((vm.count(opt1) != 0) && !vm[opt1].defaulted() && (vm.count(opt2) != 0) &&
        !vm[opt2].defaulted()) {
        throw std::logic_error(
            fmt::format("Options '{}' and '{}' cannot be used together", opt1, opt2));
    }
}

void ConflictingOptions(const po::variables_map &vm, const ConflictingOptionsVec &cov) {
    for (auto &[a, b] : cov) {
        ConflictingOptions(vm, a.c_str(), b.c_str());
    }
}

} // namespace emu::program_options
