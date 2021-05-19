#include "emu_core/program.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <stdexcept>

namespace emu {

NearOffset_t RelativeJumpOffset(Address_t position, Address_t target) {
    auto off = static_cast<int>(target) - static_cast<int>(position);
    using Limit = std::numeric_limits<NearOffset_t>;
    if (off > Limit::max() || off < Limit::min()) {
        throw std::runtime_error("Jump is too far"); //TODO
    }
    return static_cast<NearOffset_t>(off);
}

//-----------------------------------------------------------------------------

bool LabelInfo::operator==(const LabelInfo &other) const {
    if (name != other.name || imported != other.imported || offset.has_value() != other.offset.has_value()) {
        return false;
    }

    if (offset.has_value() && offset.value() != other.offset.value()) {
        return false;
    }

    return true;
}

std::string to_string(const LabelInfo &label) {
    std::string r = "LabelInfo:{";
    std::string str_off = label.offset.has_value() ? fmt::format("{:04x}", label.offset.value()) : std::string("----");
    r += fmt::format("offset:{},imported:{},name:'{}'", str_off, label.imported, label.name);
    r += "}";
    return r;
}

std::string to_string(std::weak_ptr<LabelInfo> label) {
    auto ptr = label.lock();
    if (!ptr) {
        return "LabelInfo:NULL";
    }
    return to_string(*ptr);
}

//-----------------------------------------------------------------------------

std::string to_string(RelocationMode rel_mode) {
    switch (rel_mode) {
    case RelocationMode::Absolute:
        return "Absolute";
    case RelocationMode::Relative:
        return "Relative";
    }

    throw std::runtime_error(fmt::format("Unknown RelocationMode {}", static_cast<int>(rel_mode)));
}

bool RelocationInfo::operator==(const RelocationInfo &other) const {
    if (position != other.position || mode != other.mode) {
        return false;
    }

    auto my_label = target_label.lock();
    auto oth_label = other.target_label.lock();
    if (static_cast<bool>(my_label) != static_cast<bool>(oth_label)) {
        return false;
    }
    if (my_label) {
        return *my_label == *oth_label;
    } else {
        return true;
    }
}

bool RelocationInfo::operator<(const RelocationInfo &other) const {
    if (position < other.position) {
        //  && mode < other.mode) {
        return true;
    }

    return false;

    // auto my_label = target_label.lock();
    // auto oth_label = other.target_label.lock();
    // if (static_cast<bool>(my_label) != static_cast<bool>(oth_label)) {
    //     return false;
    // }
    // if (my_label) {
    //     return my_label->name < oth_label->name;
    // } else {
    //     return true;
    // }
}

std::string to_string(const RelocationInfo &relocation) {
    std::string r = "RelocationInfo:{";
    auto label = relocation.target_label.lock();
    std::string label_name = label ? fmt::format("'{}'", label->name) : std::string("-");
    r += fmt::format("position:{:04x},mode:{},label:{}", relocation.position, to_string(relocation.mode), label_name);
    r += "}";
    return r;
}

std::string to_string(std::weak_ptr<RelocationInfo> relocation) {
    auto ptr = relocation.lock();
    if (!ptr) {
        return "RelocationInfo:NULL";
    }
    return to_string(*ptr);
}

//-----------------------------------------------------------------------------

std::pair<Address_t, Address_t> SparseBinaryCode::CodeRange() const {
    auto [min, max] =
        std::minmax_element(sparse_map.begin(), sparse_map.end(), [](auto &a, auto &b) { return a.first < b.first; });
    return {min->first, max->first};
}

void SparseBinaryCode::PutByte(Address_t address, uint8_t byte, bool overwrite) {
    if (sparse_map.find(address) != sparse_map.end() && !overwrite) {
        throw std::runtime_error(fmt::format("Address {:04x} is already occupied", address));
    }
    sparse_map[address] = byte;
}

void SparseBinaryCode::PutBytes(Address_t address, const std::vector<uint8_t> &bytes, bool overwrite) {
    for (size_t i = 0; i < bytes.size(); i++) {
        size_t a = address;
        a += i;
        if (a >= std::numeric_limits<Address_t>::max()) {
            throw std::runtime_error(fmt::format("Program::PutBytes address overflow!"));
        }
        PutByte(static_cast<Address_t>(a), bytes[i], overwrite);
    }
}

bool SparseBinaryCode::operator==(const SparseBinaryCode &other) const {
    return sparse_map == other.sparse_map;
}

std::string SparseBinaryCode::HexDump(const std::string &line_prefix) const {
    if (sparse_map.empty()) {
        return "";
    }

    auto [raw_min, raw_max] = CodeRange();
    size_t min = raw_min & 0xFFF0;
    size_t max = raw_max | 0x000F;

    std::string r;
    for (size_t pos = min; pos < max; pos += 0x10) {
        std::string hexes;
        bool any_byte = false;
        for (size_t off = 0; off <= 0xF; ++off) {
            auto it = sparse_map.find(static_cast<Address_t>(pos + off));
            if (it == sparse_map.end()) {
                hexes += " --";
            } else {
                any_byte = true;
                hexes += fmt::format(" {:02x}", it->second);
            }
        }
        if (any_byte) {
            r += fmt::format("{}{:04x} |{}\n", line_prefix, pos, hexes);
        }
    }

    return r;
}

//-----------------------------------------------------------------------------

std::string to_string(const Program &program) {
    std::string r = "Program:\n";
    r += "\tLabels:\n";
    for (auto &[n, l] : program.labels) {
        r += fmt::format("\t\t{}\n", to_string(*l));
    }
    r += "\tRelocations:\n";
    for (auto &l : program.relocations) {
        r += fmt::format("\t\t{}\n", to_string(*l));
    }
    r += "\tCode:\n";
    r += program.sparse_binary_code.HexDump("\t\t");
    r += "";
    return r;
}

bool Program::operator==(const Program &other) const {
    if (sparse_binary_code != other.sparse_binary_code) {
        return false;
    }

    for (auto &item : labels) {
        auto other_it = other.labels.find(item.first);
        if (other_it == other.labels.end() || (*other_it->second) != (*item.second)) {
            return false;
        }
    }

    if (!std::equal(relocations.begin(), relocations.end(), other.relocations.begin(), other.relocations.end(),
                    [](auto &a, auto &b) { return (*a) == (*b); })) {
        return false;
    }

    return true;
}

} // namespace emu
