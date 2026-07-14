#include "leaderrank.h"

#include "common.h"
#include "graph_store.h"
#include "parallel.h"

#include <atomic>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace gra {

namespace {

bool WriteRanks(const std::string &path, const std::vector<double> &rank,
                const std::vector<Vertex> &id_map) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << "vertex,rank\n" << std::setprecision(10);
    for (size_t i = 0; i < rank.size(); ++i) {
        out << id_map[i] << ',' << rank[i] << '\n';
    }
    return !out.fail();
}

}  // namespace

int RunLeaderRank(const LeaderRankOptions &opts) {
    GraphMeta meta;
    if (!LoadMeta(opts.input_dir, meta)) {
        std::cerr << "error: cannot read meta.txt in " << opts.input_dir << "\n";
        return 1;
    }
    size_t n = meta.n;

    std::vector<Degree> outdeg;
    if (!LoadOutDegrees(opts.input_dir, n, outdeg)) {
        std::cerr << "error: cannot read outdeg.bin\n";
        return 1;
    }
    std::vector<Vertex> id_map;
    if (!LoadIdMap(opts.input_dir, n, id_map)) {
        std::cerr << "error: cannot read id_map.bin\n";
        return 1;
    }

    int threads = ResolveThreads(opts.num_threads);
    double inv_n = 1.0 / n;

    std::vector<double> cur(n, 1.0);
    std::vector<double> next(n, 0.0);
    double sg_cur = 0.0;

    double diff = 0.0;
    for (int it = 0; it < opts.max_iterations; ++it) {
        double base = sg_cur * inv_n;

        double sg_next = 0.0;
        for (size_t i = 0; i < n; ++i) {
            sg_next += cur[i] / (outdeg[i] + 1);
            next[i] = base;
        }

        std::atomic<bool> failed{false};
        ParallelShards(meta.num_shards, threads, [&](int s) {
            MappedFile shard(ShardPath(opts.input_dir, s));
            if (!shard.Ok()) {
                failed = true;
                return;
            }
            const Vertex *p = static_cast<const Vertex *>(shard.Data());
            size_t m = shard.Size() / (2 * sizeof(Vertex));
            for (size_t k = 0; k < m; ++k) {
                Vertex u = p[2 * k];
                Vertex v = p[2 * k + 1];
                next[v] += cur[u] / (outdeg[u] + 1);
            }
        });
        if (failed) {
            std::cerr << "error: cannot map shard\n";
            return 1;
        }

        diff = std::fabs(sg_next - sg_cur);
        for (size_t i = 0; i < n; ++i) {
            diff += std::fabs(next[i] - cur[i]);
        }

        cur.swap(next);
        sg_cur = sg_next;

        if (diff < opts.tolerance * n) {
            break;
        }
    }

    double add = sg_cur * inv_n;
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i) {
        cur[i] += add;
        sum += cur[i];
    }
    double inv = 1.0 / sum;
    for (size_t i = 0; i < n; ++i) {
        cur[i] *= inv;
    }

    if (!WriteRanks(opts.output_path, cur, id_map)) {
        std::cerr << "error: cannot write output " << opts.output_path << "\n";
        return 1;
    }
    return 0;
}

}  // namespace gra
