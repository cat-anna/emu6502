#pragma once

#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace emu6502::assembler {

using Address_t = uint16_t;
using Offset_t = int16_t;

struct RelocationInfo;

struct LabelInfo {
    std::string name;
    std::optional<Address_t> offset;
    bool imported = false;

    std::vector<std::weak_ptr<RelocationInfo>> label_references;

    bool operator==(const LabelInfo &other) const;
};

std::string to_string(const LabelInfo &label);
std::string to_string(std::weak_ptr<LabelInfo> label);

//-----------------------------------------------------------------------------

enum class RelocationMode {
    Absolute,
    Relative,
};
std::string to_string(RelocationMode rel_mode);

struct RelocationInfo {
    std::weak_ptr<LabelInfo> target_label;
    Address_t position;
    RelocationMode mode;

    bool operator==(const RelocationInfo &other) const;
};

std::string to_string(const RelocationInfo &relocation);
std::string to_string(std::weak_ptr<RelocationInfo> relocation);

//-----------------------------------------------------------------------------

struct SparseBinaryCode {
    using MapType = std::unordered_map<Address_t, uint8_t>;
    using VectorType = std::vector<uint8_t>;
    MapType sparse_map;

    SparseBinaryCode(std::initializer_list<MapType::value_type> init) : sparse_map(std::move(init)) {}
    SparseBinaryCode() = default;
    SparseBinaryCode(Address_t base_address, const VectorType &bytes) { PutBytes(base_address, bytes); }
    SparseBinaryCode(const VectorType &bytes) : SparseBinaryCode(0, bytes) {}

    std::pair<Address_t, Address_t> CodeRange() const;
    void PutByte(Address_t address, uint8_t byte, bool overwrite = false);
    void PutBytes(Address_t address, const std::vector<uint8_t> &bytes, bool overwrite = false);

    std::string HexDump(const std::string &line_prefix = "") const;

    bool operator==(const SparseBinaryCode &other) const;
};

//-----------------------------------------------------------------------------

struct Program {
    SparseBinaryCode sparse_binary_code;
    std::unordered_map<std::string, std::shared_ptr<LabelInfo>> labels;
    std::set<std::shared_ptr<RelocationInfo>> relocations;

    bool operator==(const Program &other) const;
};

std::string to_string(const Program &program);

} // namespace emu6502::assembler
