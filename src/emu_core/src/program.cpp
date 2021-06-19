#include "emu_core/program.hpp"
#include "emu_core/base16.hpp"
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

uint8_t RelocationSize(RelocationMode rm) {
    switch (rm) {
    case RelocationMode::Absolute:
        return 2;
    case RelocationMode::Relative:
        return 1;
    }
    throw std::runtime_error(fmt::format("Unknown relocation mode {}", static_cast<int>(rm)));
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

std::string to_string(const ValueAlias &value_alias) {
    return fmt::format("ValueAlias{{name={},value=[{}]}}", value_alias.name, ToHex(value_alias.value, ""));
}

std::string to_string(std::shared_ptr<ValueAlias> value_alias) {
    return to_string(*value_alias);
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

std::string SparseBinaryCode::HexDump(std::string_view line_prefix) const {
    return SparseHexDump(sparse_map, line_prefix);
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
    r += "\tAliases:\n";
    for (auto &[n, l] : program.aliases) {
        r += fmt::format("\t\t{}\n", to_string(*l));
    }
    r += "\tCode:\n";
    r += program.sparse_binary_code.HexDump("\t\t");
    r += "";
    return r;
}

std::ostream &operator<<(std::ostream &o, const Program &program) {
    return o << to_string(program);
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
std::shared_ptr<ValueAlias> Program::FindAlias(const std::string &name) const {
    auto it = aliases.find(name);
    return it == aliases.end() ? nullptr : it->second;
}

std::shared_ptr<LabelInfo> Program::FindLabel(const std::string &name) const {
    auto it = labels.find(name);
    return it == labels.end() ? nullptr : it->second;
}

void Program::AddAlias(std::shared_ptr<ValueAlias> alias) {
    if (alias->name.size() < 2) {
        throw std::runtime_error(fmt::format("Label '{}' name is to short", alias->name));
    }
    if (aliases.contains(alias->name)) {
        throw std::runtime_error(fmt::format("Alias '{}' is already defined", alias->name));
    }
    aliases[alias->name] = std::move(alias);
}

void Program::AddLabel(std::shared_ptr<LabelInfo> label) {
    if (label->name.size() < 2) {
        throw std::runtime_error(fmt::format("Label '{}' name is to short", label->name));
    }
    if (labels.contains(label->name)) {
        throw std::runtime_error(fmt::format("Label '{}' is already defined", label->name));
    }
    labels[label->name] = std::move(label);
}

} // namespace emu
