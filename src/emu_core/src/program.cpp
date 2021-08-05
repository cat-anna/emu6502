#include "emu_core/program.hpp"
#include "emu_core/base16.hpp"
#include "emu_core/byte_utils.hpp"
#include <algorithm>
#include <fmt/format.h>
#include <limits>
#include <stdexcept>

namespace emu {

NearOffset_t RelativeJumpOffset(Address_t position, Address_t target) {
    auto off = static_cast<int>(target) - static_cast<int>(position);
    using Limit = std::numeric_limits<NearOffset_t>;
    if (off > Limit::max() || off < Limit::min()) {
        throw std::runtime_error(
            fmt::format("Relative jump {:04x}->{:04x} is too far", position, target));
    }
    return static_cast<NearOffset_t>(off);
}

std::vector<uint8_t> ToBytes(const SymbolAddress &v,
                             std::optional<size_t> expected_size) {
    auto r = std::visit([](auto &i) { return ToBytes(i); }, v);
    if (expected_size.has_value() && expected_size.value() != r.size()) {
        throw std::runtime_error("Symbol address does not have expected size");
    }
    return r;
}

//-----------------------------------------------------------------------------

bool SymbolInfo::operator==(const SymbolInfo &other) const {
    return name == other.name && imported == other.imported && offset == other.offset;
}

std::string to_string(const SymbolInfo &symbol) {
    std::string r = "SymbolInfo:{";
    r += fmt::format("name:'{}',", symbol.name);
    if (HasValue(symbol.offset)) {
        r += fmt::format(",offset:{},",
                         FormatHex(ToBytes(symbol.offset, std::nullopt), ""));
    }
    if (symbol.segment.has_value()) {
        r += fmt::format(",segment:{}", to_string(symbol.segment.value()));
    }
    r += fmt::format(",imported:{}", symbol.imported);
    r += "}";
    return r;
}

std::string to_string(std::weak_ptr<SymbolInfo> symbol) {
    auto ptr = symbol.lock();
    if (!ptr) {
        return "SymbolInfo:NULL";
    }
    return to_string(*ptr);
}

std::string to_string(Segment mode) {
    switch (mode) {
    case Segment::ZeroPage:
        return "ZeroPage";
    case Segment::Code:
        return "Code";
    case Segment::Data:
        return "Data";
    case Segment::RoData:
        return "RoData";
    case Segment::AbsoluteAddress:
        return "AbsoluteAddress";
    }

    throw std::runtime_error(fmt::format("Unknown Segment {}", static_cast<int>(mode)));
}

bool Relocable(Segment mode) {
    switch (mode) {
    case Segment::ZeroPage:
    case Segment::Code:
    case Segment::Data:
    case Segment::RoData:
        return true;
    case Segment::AbsoluteAddress:
        return false;
    }
    throw std::runtime_error(fmt::format("Unknown Segment {}", static_cast<int>(mode)));
}

//-----------------------------------------------------------------------------

std::string to_string(RelocationMode rel_mode) {
    switch (rel_mode) {
    case RelocationMode::Absolute:
        return "Absolute";
    case RelocationMode::Relative:
        return "Relative";
    case RelocationMode::ZeroPage:
        return "ZeroPage";
    }

    throw std::runtime_error(
        fmt::format("Unknown RelocationMode {}", static_cast<int>(rel_mode)));
}

uint8_t RelocationSize(RelocationMode rm) {
    switch (rm) {
    case RelocationMode::Absolute:
        return 2;
    case RelocationMode::Relative:
    case RelocationMode::ZeroPage:
        return 1;
    }
    throw std::runtime_error(
        fmt::format("Unknown relocation mode {}", static_cast<int>(rm)));
}

bool RelocationInfo::operator==(const RelocationInfo &other) const {
    if (position != other.position || mode != other.mode) {
        return false;
    }

    auto my_symbol = target_symbol.lock();
    auto oth_symbol = other.target_symbol.lock();
    if (static_cast<bool>(my_symbol) != static_cast<bool>(oth_symbol)) {
        return false;
    }
    if (my_symbol) {
        return *my_symbol == *oth_symbol;
    } else {
        return true;
    }
}

bool RelocationInfo::operator<(const RelocationInfo &other) const {
    return position < other.position;
}

std::string to_string(const RelocationInfo &relocation) {
    std::string r = "RelocationInfo:{";
    auto symbol = relocation.target_symbol.lock();
    std::string symbol_name =
        symbol ? fmt::format("'{}'", symbol->name) : std::string("-");
    r += fmt::format("position:{:04x},mode:{},symbol:{}", relocation.position,
                     to_string(relocation.mode), symbol_name);
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
    return fmt::format("ValueAlias{{name={},value=[{}]}}", value_alias.name,
                       ToHex(value_alias.value, ""));
}

std::string to_string(std::shared_ptr<ValueAlias> value_alias) {
    return to_string(*value_alias);
}

//-----------------------------------------------------------------------------

std::pair<Address_t, Address_t> SparseBinaryCode::CodeRange() const {
    auto [min, max] =
        std::minmax_element(sparse_map.begin(), sparse_map.end(),
                            [](auto &a, auto &b) { return a.first < b.first; });
    return {min->first, max->first};
}

void SparseBinaryCode::PutByte(Address_t address, uint8_t byte, bool overwrite) {
    if (sparse_map.find(address) != sparse_map.end() && !overwrite) {
        throw std::runtime_error(
            fmt::format("Address {:04x} is already occupied", address));
    }
    sparse_map[address] = byte;
}

void SparseBinaryCode::PutBytes(Address_t address, const std::vector<uint8_t> &bytes,
                                bool overwrite) {
    for (size_t i = 0; i < bytes.size(); i++) {
        size_t a = address;
        a += i;
        if (a > std::numeric_limits<Address_t>::max()) {
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

ByteVector SparseBinaryCode::DumpMemory() const {
    ByteVector r(0x10000, 0);

    for (auto [addr, value] : sparse_map) {
        r[addr] = value;
    }

    return r;
}

//-----------------------------------------------------------------------------

std::string to_string(const Program &program) {
    std::string r = "Program:\n";
    r += "\tsymbols:\n";
    for (auto &[n, l] : program.symbols) {
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

    for (auto &item : symbols) {
        auto other_it = other.symbols.find(item.first);
        if (other_it == other.symbols.end() || (*other_it->second) != (*item.second)) {
            return false;
        }
    }

    if (!std::equal(relocations.begin(), relocations.end(), other.relocations.begin(),
                    other.relocations.end(),
                    [](auto &a, auto &b) { return (*a) == (*b); })) {
        return false;
    }

    return true;
}
std::shared_ptr<ValueAlias> Program::FindAlias(const std::string &name) const {
    auto it = aliases.find(name);
    return it == aliases.end() ? nullptr : it->second;
}

std::shared_ptr<SymbolInfo> Program::FindSymbol(const std::string &name) const {
    auto it = symbols.find(name);
    return it == symbols.end() ? nullptr : it->second;
}

void Program::AddAlias(std::shared_ptr<ValueAlias> alias) {
    if (alias->name.size() < 2) {
        throw std::runtime_error(
            fmt::format("symbol '{}' name is to short", alias->name));
    }
    if (aliases.contains(alias->name)) {
        throw std::runtime_error(
            fmt::format("Alias '{}' is already defined", alias->name));
    }
    aliases[alias->name] = std::move(alias);
}

void Program::AddSymbol(std::shared_ptr<SymbolInfo> symbol) {
    if (symbol->name.size() < 2) {
        throw std::runtime_error(
            fmt::format("symbol '{}' name is to short", symbol->name));
    }
    if (symbols.contains(symbol->name)) {
        throw std::runtime_error(
            fmt::format("symbol '{}' is already defined", symbol->name));
    }
    symbols[symbol->name] = std::move(symbol);
}

} // namespace emu
