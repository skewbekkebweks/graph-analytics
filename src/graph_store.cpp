#include "graph_store.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace gra {

bool LoadMeta(const std::string &dir, GraphMeta &meta) {
    std::ifstream in((fs::path(dir) / "meta.txt").string());
    if (!in) {
        return false;
    }
    std::string key;
    in >> key >> meta.n >> key >> meta.edges >> key >> meta.num_shards >> key >> meta.shard_width;
    return !in.fail();
}

bool LoadOutDegrees(const std::string &dir, std::uint64_t n, std::vector<Degree> &outdeg) {
    std::ifstream in((fs::path(dir) / "outdeg.bin").string(), std::ios::binary);
    if (!in) {
        return false;
    }
    outdeg.resize(n);
    in.read(reinterpret_cast<char *>(outdeg.data()), n * sizeof(Degree));
    return !in.fail();
}

bool LoadIdMap(const std::string &dir, std::uint64_t n, std::vector<Vertex> &id_map) {
    std::ifstream in((fs::path(dir) / "id_map.bin").string(), std::ios::binary);
    if (!in) {
        return false;
    }
    id_map.resize(n);
    in.read(reinterpret_cast<char *>(id_map.data()), n * sizeof(Vertex));
    return !in.fail();
}

std::string ShardPath(const std::string &dir, int index) {
    char name[32];
    std::snprintf(name, sizeof(name), "shard_%04d.bin", index);
    return (fs::path(dir) / name).string();
}

MappedFile::MappedFile(const std::string &path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        return;
    }
    struct stat st{};
    if (fstat(fd, &st) != 0) {
        close(fd);
        return;
    }
    size_ = st.st_size;
    ok_ = true;
    if (size_ > 0) {
        data_ = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data_ == MAP_FAILED) {
            data_ = nullptr;
            size_ = 0;
            ok_ = false;
        }
    }
    close(fd);
}

void MappedFile::Release() {
    if (data_ != nullptr && size_ > 0) {
        munmap(data_, size_);
    }
    data_ = nullptr;
    size_ = 0;
    ok_ = false;
}

MappedFile::~MappedFile() {
    Release();
}

MappedFile::MappedFile(MappedFile &&other) noexcept
    : data_(other.data_), size_(other.size_), ok_(other.ok_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.ok_ = false;
}

MappedFile &MappedFile::operator=(MappedFile &&other) noexcept {
    if (this != &other) {
        Release();
        data_ = other.data_;
        size_ = other.size_;
        ok_ = other.ok_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.ok_ = false;
    }
    return *this;
}

bool MappedFile::Ok() const {
    return ok_;
}

const void *MappedFile::Data() const {
    return data_;
}

size_t MappedFile::Size() const {
    return size_;
}

}  // namespace gra
