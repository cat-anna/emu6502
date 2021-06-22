#pragma once

#include <concepts>
#include <cstdint>
#include <fmt/format.h>
#include <stdexcept>
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
};

using MemoryMock16 = MemoryInterfaceMock<uint16_t>;

} // namespace emu

#endif
