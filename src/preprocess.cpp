#include "preprocess.h"

#include "common.h"
#include "csv_reader.h"
#include "graph_store.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>

namespace fs = std::filesystem;

namespace gra {

namespace {

template <class T>
bool WriteVector(const std::string &path, const std::vector<T> &data) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char *>(data.data()), data.size() * sizeof(T));
    return !out.fail();
}

bool WriteMeta(const std::string &dir, std::uint64_t n, std::uint64_t edges, int num_shards,
               std::uint64_t shard_width) {
    std::ofstream out((fs::path(dir) / "meta.txt").string());
    if (!out) {
        return false;
    }
    out << "n " << n << "\nedges " << edges << "\nshards " << num_shards << "\nshard_width "
        << shard_width << "\n";
    return !out.fail();
}

}  // namespace

int RunPreprocess(const PreprocessOptions &opts) {
    std::error_code ec;
    fs::create_directories(opts.output_dir, ec);

    std::vector<Vertex> raw_to_dense;
    std::vector<Degree> raw_outdeg;
    std::vector<Degree> raw_indeg;
    std::uint64_t edge_count = 0;
    CsvEdgeReader reader(opts.input_path);
    if (!reader.IsOpen()) {
        std::cerr << "error: cannot open input " << opts.input_path << "\n";
        return 1;
    }
    Vertex u = 0;
    Vertex v = 0;
    while (reader.Next(u, v)) {
        Vertex max_id = std::max(u, v);
        if (max_id >= raw_to_dense.size()) {
            raw_to_dense.resize(max_id + 1, 0);
            raw_outdeg.resize(max_id + 1, 0);
            raw_indeg.resize(max_id + 1, 0);
        }
        raw_to_dense[u] = 1;
        raw_to_dense[v] = 1;
        raw_outdeg[u] += 1;
        raw_indeg[v] += 1;
        ++edge_count;
    }

    if (raw_to_dense.empty()) {
        std::cerr << "error: empty graph\n";
        return 1;
    }

    std::vector<Vertex> id_map;
    std::vector<Degree> outdeg;
    std::vector<Degree> indeg;
    for (size_t i = 0; i < raw_to_dense.size(); ++i) {
        if (raw_to_dense[i] == 1) {
            raw_to_dense[i] = static_cast<Vertex>(id_map.size());
            id_map.push_back(static_cast<Vertex>(i));
            outdeg.push_back(raw_outdeg[i]);
            indeg.push_back(raw_indeg[i]);
        }
    }
    raw_outdeg.clear();
    raw_outdeg.shrink_to_fit();
    raw_indeg.clear();
    raw_indeg.shrink_to_fit();

    std::uint64_t n = id_map.size();

    if (!WriteVector((fs::path(opts.output_dir) / "outdeg.bin").string(), outdeg) ||
        !WriteVector((fs::path(opts.output_dir) / "id_map.bin").string(), id_map)) {
        std::cerr << "error: cannot write output files\n";
        return 1;
    }

    int num_shards = std::min(std::max(1, opts.num_shards), static_cast<int>(n));

    std::vector<int> shard_of(n);
    std::uint64_t acc = 0;
    for (size_t d = 0; d < n; ++d) {
        shard_of[d] = std::min(static_cast<int>(acc * num_shards / edge_count), num_shards - 1);
        acc += indeg[d];
    }

    std::vector<std::ofstream> shards(num_shards);
    for (int i = 0; i < num_shards; ++i) {
        shards[i].open(ShardPath(opts.output_dir, i), std::ios::binary);
        if (!shards[i]) {
            std::cerr << "error: cannot open shard " << i << "\n";
            return 1;
        }
    }
    reader = CsvEdgeReader(opts.input_path);
    if (!reader.IsOpen()) {
        std::cerr << "error: cannot reopen input " << opts.input_path << "\n";
        return 1;
    }
    u = 0;
    v = 0;
    while (reader.Next(u, v)) {
        Vertex edge[2] = {raw_to_dense[u], raw_to_dense[v]};
        shards[shard_of[edge[1]]].write(reinterpret_cast<const char *>(edge), sizeof(edge));
    }

    if (!WriteMeta(opts.output_dir, n, edge_count, num_shards, 0)) {
        std::cerr << "error: cannot write meta.txt\n";
        return 1;
    }

    return 0;
}

}  // namespace gra
