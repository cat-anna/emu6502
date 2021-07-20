#pragma once

#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace emu {

enum class MemoryMode {
    kReadOnly = 1,
    kReadWrite,
    kThrowOnWrite,
};

class MemoryOutOfBoundAccessException : public std::runtime_error {
public:
    MemoryOutOfBoundAccessException(uint64_t offset, uint64_t limit,
                                    const std::string &msg)
        : std::runtime_error(
              fmt::format("Memory out of bound access error: {}: offset={:x} limit={:x}",
                          msg, offset, limit)) {}
};

class MemoryWriteAttemptException : public std::runtime_error {
public:
    MemoryWriteAttemptException(uint64_t offset, uint64_t limit, const std::string &msg)
        : std::runtime_error(
              fmt::format("Memory write attempt error: {}: offset={:x} limit={:x}", msg,
                          offset, limit)) {}
};
template <std::unsigned_integral _Address_t>
class MemoryInterface {
public:
    using Address_t = _Address_t;

    virtual ~MemoryInterface() = default;

    [[nodiscard]] virtual uint8_t Load(Address_t address) const = 0;
    virtual void Store(Address_t address, uint8_t value) = 0;

    [[nodiscard]] virtual MemoryMode Mode() const {
        throw std::runtime_error("MemoryInterface::Mode() is not implemented");
    }

    [[nodiscard]] virtual std::optional<uint8_t> DebugRead(Address_t address) const = 0;

    [[nodiscard]] virtual std::vector<std::optional<uint8_t>>
    DebugReadRange(Address_t address, size_t len) const {
        std::vector<std::optional<uint8_t>> r;
        r.reserve(len);
        for (size_t beg = address, end = beg + len; beg < end; ++beg) {
            r.emplace_back(DebugRead(static_cast<Address_t>(beg)));
        }
        return r;
    }

    static void WriteAccessLog(std::ostream &out, const char *memtype,
                               const std::string &name, bool w, Address_t address,
                               uint8_t value, const char *comment = "") {
        out << fmt::format("MEM-{:8s} [{:8}] {} [{:04x}] {} {:02x} {}\n", memtype, name,
                           (w ? "W" : "R"), address, (w ? "<-" : "->"), value, comment);
    }
};

using Memory16 = MemoryInterface<uint16_t>;

} // namespace emu

#ifdef WANTS_GTEST_MOCKS

#include <gmock/gmock.h>
namespace emu {
template <typename _Address_t>
struct MemoryInterfaceMock : public MemoryInterface<_Address_t> {
    using Address_t = _Address_t;
    MOCK_METHOD(uint8_t, Load, (Address_t), (const));
    MOCK_METHOD(void, Store, (Address_t, uint8_t));
    MOCK_METHOD(std::optional<uint8_t>, DebugRead, (Address_t), (const));
    MOCK_METHOD(std::vector<std::optional<uint8_t>>, DebugReadRange, (Address_t, size_t),
                (const));
};

using MemoryMock16 = MemoryInterfaceMock<uint16_t>;

} // namespace emu

#endif
