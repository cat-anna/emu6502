#include "text_utils.hpp"
#include <charconv>
#include <cinttypes>
#include <fmt/format.h>
#include <limits>
#include <regex>
#include <stdexcept>

namespace emu::assembler {

std::vector<std::string_view> Tokenize(std::string_view line) {
    std::vector<std::string_view> r;

    while (!line.empty()) {
        if (isspace(line[0])) {
            line.remove_prefix(1);
            continue;
        }

        auto pos = line.find_first_of("\t\n ;,");
        if (pos == std::string_view::npos) {
            pos = line.size();
        }

        auto token = line.substr(0, pos);
        if (!token.empty()) {
            r.emplace_back(token);
        }

        line.remove_prefix(pos);
        if (!line.empty()) {
            if (line[0] == ';') {
                break;
            }
            line.remove_prefix(1);
        }
    }

    return r;
}

uint8_t ParseByte(std::string_view text, int base) {
    auto raw = std::stol(text.data(), 0, base);
    if (raw > std::numeric_limits<uint8_t>::max() || raw < std::numeric_limits<uint8_t>::min()) {
        throw std::runtime_error(fmt::format("Cannot parse {} into byte", text));
    }
    return static_cast<uint8_t>(raw);
}

uint16_t ParseWord(std::string_view text, int base) {
    auto raw = std::stol(text.data(), 0, base);
    if (raw > std::numeric_limits<uint16_t>::max() || raw < std::numeric_limits<uint16_t>::min()) {
        throw std::runtime_error(fmt::format("Cannot parse {} into byte", text));
    }
    return static_cast<uint16_t>(raw);
}

InstructionArgument ParseInstructionArgument(std::string_view arg) {
    using AddressMode = emu::cpu6502::opcode::AddressMode;
    // +---------------------+--------------------------+
    // |      mode           |     assembler format     |
    // +=====================+==========================+

    // | Implied             |                          |
    if (arg.empty()) {
        return InstructionArgument{
            .possible_address_modes = {AddressMode::Implied},
            .argument_value = nullptr,
        };
    }

    // | Accumulator         |          A               |
    if (arg == "A") {
        return InstructionArgument{
            .possible_address_modes = {AddressMode::ACC},
            .argument_value = nullptr,
        };
    }

    using AM = AddressMode;
    const std::vector<std::tuple<std::regex, std::set<AM>>> fmt_regex = {
        // | Immediate           |          #aa             |
        {std::regex{R"==(^#([$\w]+)$)=="}, {AM::Immediate}},

        // | Absolute            |          aaaa            |
        // | Zero Page           |          aa              |
        // | Relative            |          aaaa            |
        {std::regex{R"==(^([$\w]+)$)=="}, {AM::ABS, AM::ZP, AM::REL}},

        // | Indirect Absolute   |          (aaaa)          |
        {std::regex{R"==(^\(([$\w]+)\)$)=="}, {AM::ABS_IND}},

        // | Zero Page Indexed,X |          aa,X            |
        // | Absolute Indexed,X  |          aaaa,X          |
        {std::regex{R"==(^([$\w]+),X$)=="}, {AM::ABSX, AM::ZPX}},

        // | Zero Page Indexed,Y |          aa,Y            |
        // | Absolute Indexed,Y  |          aaaa,Y          |
        {std::regex{R"==(^([$\w]+),Y$)=="}, {AM::ABSY, AM::ZPY}},

        // | Indexed Indirect    |          (aa,X)          |
        {std::regex{R"==(^\(([$\w]+),X\)$)=="}, {AM::INDX}},

        // | Indirect Indexed    |          (aa),Y          |
        {std::regex{R"==(^\(([$\w]+)\),Y$)=="}, {AM::INDY}},
    };

    for (const auto &[regex, set] : fmt_regex) {
        std::smatch match;
        auto str = std::string(arg);
        if (std::regex_match(str, match, regex)) {
            std::string s_value = match[1];
            auto value = std::string_view(s_value);

            if (value.empty()) {
                //TODO:
                throw std::runtime_error("value.empty()");
            }

            if (value.starts_with("$")) {
                value.remove_prefix(1);
                std::vector<uint8_t> data;
                switch (value.size()) {
                case 2:
                    data = ToBytes(ParseByte(value, 16));
                    break;
                case 4:
                    data = ToBytes(ParseWord(value, 16));
                    break;
                default:
                    throw std::runtime_error("Invalid literal value");
                }
                auto possible_address_modes = set;
                possible_address_modes.erase(AM::REL);
                if (data.size() == 1) {
                    possible_address_modes.erase(AM::ABSX);
                    possible_address_modes.erase(AM::ABSY);
                    possible_address_modes.erase(AM::ABS);
                } else {
                    possible_address_modes.erase(AM::ZPX);
                    possible_address_modes.erase(AM::ZPY);
                    possible_address_modes.erase(AM::ZP);
                }
                if (possible_address_modes.empty()) {
                    throw std::runtime_error("Impossible literal size");
                }
                for (auto i : possible_address_modes) {
                    if (emu::cpu6502::opcode::ArgumentByteSize(i) != data.size()) {
                        throw std::runtime_error("Invalid literal size");
                    }
                }
                return InstructionArgument{
                    .possible_address_modes = {possible_address_modes.begin(), possible_address_modes.end()},
                    .argument_value = data,
                };
            } else {
                auto possible_address_modes = set;
                return InstructionArgument{
                    .possible_address_modes = {possible_address_modes.begin(), possible_address_modes.end()},
                    .argument_value = s_value,
                };
            }
        }
    }

    throw std::runtime_error("not implemted");
}

} // namespace emu::assembler
