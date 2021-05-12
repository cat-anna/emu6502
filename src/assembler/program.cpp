#include "program.hpp"
#include <algorithm>
#include <fmt/format.h>

namespace emu6502::assembler {

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
    std::string str_off = label.offset.has_value() ? std::to_string(label.offset.value()) : std::string("--");
    r += fmt::format("N:'{}',Off:{},imported:{}", label.name, str_off, label.imported);
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

std::string to_string(const RelocationInfo &relocation) {
    std::string r = "RelocationInfo:{";
    auto label = relocation.target_label.lock();
    std::string label_name = label ? label->name : std::string("-");
    r += fmt::format("L:'{}',Pos:{},Mode:{}", label_name, relocation.position, relocation.mode);
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
    for (size_t pos = min; pos <= max; pos += 16) {
        std::string hexes;
        for (size_t off = 0; off < 16; ++off) {
            auto it = sparse_map.find(pos + off);
            if (it == sparse_map.end()) {
                hexes += " --";
            } else {
                hexes += fmt::format(" {:02x}", it->second);
            }
        }
        r += fmt::format("{}{:04x} |{}", line_prefix, pos, hexes);
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
                    [](auto &a, auto &b) { return *a == *b; })) {
        return false;
    }

    return true;
}

} // namespace emu6502::assembler
