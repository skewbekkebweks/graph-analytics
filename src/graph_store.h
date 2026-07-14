#pragma once

#include "common.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace gra {

struct GraphMeta {
    std::uint64_t n = 0;
    std::uint64_t edges = 0;
    int num_shards = 0;
    std::uint64_t shard_width = 0;
};

bool LoadMeta(const std::string &dir, GraphMeta &meta);
bool LoadOutDegrees(const std::string &dir, std::uint64_t n, std::vector<Degree> &outdeg);
bool LoadIdMap(const std::string &dir, std::uint64_t n, std::vector<Vertex> &id_map);
std::string ShardPath(const std::string &dir, int index);

class MappedFile {
public:
    explicit MappedFile(const std::string &path);
    ~MappedFile();

    MappedFile(MappedFile &&other) noexcept;
    MappedFile &operator=(MappedFile &&other) noexcept;

    MappedFile(const MappedFile &) = delete;
    MappedFile &operator=(const MappedFile &) = delete;

    bool Ok() const;
    const void *Data() const;
    size_t Size() const;

private:
    void Release();

    void *data_ = nullptr;
    size_t size_ = 0;
    bool ok_ = false;
};

}  // namespace gra
