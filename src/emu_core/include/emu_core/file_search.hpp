#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace emu {

struct FileNotFoundException : public std::runtime_error {
    FileNotFoundException(const std::string &m, std::string long_message)
        : runtime_error(m), long_message(std::move(long_message)) {}
    const std::string long_message;
};

struct FileSearch {
    virtual ~FileSearch() = default;

    virtual std::string SearchPath(const std::string &name) const = 0;
    virtual std::shared_ptr<std::istream> OpenFile(const std::string &name,
                                                   bool binary = false) const;

    virtual std::shared_ptr<FileSearch> PrependPath(const std::string &name) const = 0;

    static std::shared_ptr<FileSearch> PrependPath(const std::string &name,
                                                   FileSearch *searcher,
                                                   std::ostream *log = &std::cout);

    static std::shared_ptr<FileSearch> CreateFromEnv(const std::string &env_var_name,
                                                     std::ostream *log = &std::cout);
    static std::shared_ptr<FileSearch> Create(const std::string &colon_separated_list,
                                              std::ostream *log = &std::cout);
    static std::shared_ptr<FileSearch> Create(std::vector<std::string> list,
                                              std::ostream *log = &std::cout);
};

} // namespace emu

#ifdef WANTS_GTEST_MOCKS

#include <gmock/gmock.h>
namespace emu {
struct FileSearchMock : public FileSearch {
    MOCK_METHOD(std::shared_ptr<FileSearch>, PrependPath, (const std::string &), (const));
    MOCK_METHOD(std::string, SearchPath, (const std::string &), (const));
    MOCK_METHOD(std::shared_ptr<std::istream>, OpenFile, (const std::string &, bool),
                (const));
};

} // namespace emu

#endif
