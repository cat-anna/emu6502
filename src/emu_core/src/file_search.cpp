#include "emu_core/file_search.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace emu {

namespace {

struct PathListSearcher : public FileSearch,
                          public std::enable_shared_from_this<PathListSearcher> {
    PathListSearcher(std::vector<std::string> list, std::ostream *log)
        : path_list(), log(log) {
        std::transform(
            list.begin(), list.end(), std::back_inserter(path_list),
            [](auto i) { return std::filesystem::absolute(std::filesystem::path(i)); });
    }

    PathListSearcher(std::vector<std::filesystem::path> list,
                     std::filesystem::path additional, std::ostream *log)
        : path_list(list), log(log) {

        path_list.insert(path_list.begin(), additional);
    }

    std::shared_ptr<FileSearch> PrependPath(const std::string &name) const override {
        auto path = std::filesystem::absolute(std::filesystem::path(name));
        if (!std::filesystem::is_directory(path)) {
            path = path.parent_path();
        }
        if (path.empty()) {
            return const_cast<PathListSearcher *>(this)->shared_from_this();
        }
        return std::make_shared<PathListSearcher>(path_list, path, log);
    }

    std::string SearchPath(const std::string &name) const override {
        auto test = [&](auto full_path, auto base_path) -> std::optional<std::string> {
            if (std::filesystem::is_regular_file(full_path)) {
                if (log != nullptr) {
                    *log << fmt::format("PathListSearcher: Found '{}' in '{}'\n", name,
                                        base_path.generic_string());
                }
                return full_path.generic_string();
            } else {
                if (log != nullptr) {
                    *log << fmt::format("PathListSearcher: '{}' not available in '{}'\n",
                                        name, base_path.generic_string());
                }
            }
            return std::nullopt;
        };

        for (auto &path : path_list) {
            auto r = test(std::filesystem::absolute(path / name), path);
            if (r.has_value()) {
                return *r;
            }
        }

        std::stringstream ss;
        ss << "PathListSearcher: Could not find '" << name << "', tried:\n";
        for (auto &item : path_list) {
            ss << "\t" << item.generic_string() << "\n";
        }
        throw FileNotFoundException(fmt::format("'{}' not found", name), ss.str());
    }

private:
    std::vector<std::filesystem::path> path_list;
    std::ostream *log;
};

} // namespace

std::shared_ptr<FileSearch> FileSearch::CreateFromEnv(const std::string &env_var_name,
                                                      std::ostream *log) {
    auto v = getenv(env_var_name.c_str());
    std::string s;
    if (v != nullptr) {
        s = v;
    }
    return Create(std::move(s), log);
}

std::shared_ptr<FileSearch> FileSearch::Create(const std::string &colon_separated_list,
                                               std::ostream *log) {
    std::vector<std::string> r;
    boost::split(r, colon_separated_list, boost::is_any_of(":"));
    return Create(r);
}

std::shared_ptr<FileSearch> FileSearch::Create(std::vector<std::string> list,
                                               std::ostream *log) {
    return std::make_shared<PathListSearcher>(std::move(list), log);
}

std::shared_ptr<std::istream> FileSearch::OpenFile(const std::string &name,
                                                   bool binary) const {
    auto path = SearchPath(name);
    auto f = std::make_shared<std::ifstream>();
    f->exceptions(std::ifstream::badbit);
    f->open(path, (binary ? std::ios::out | std::ios::binary : std::ios::out));
    return f;
}

std::shared_ptr<FileSearch> FileSearch::PrependPath(const std::string &name,
                                                    FileSearch *searcher,
                                                    std::ostream *log) {
    if (searcher != nullptr) {
        return searcher->PrependPath(name);
    }
    return std::make_shared<PathListSearcher>(std::vector<std::filesystem::path>{},
                                              std::filesystem::path(name), log);
}

} // namespace emu