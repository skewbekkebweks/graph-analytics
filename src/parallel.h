#pragma once

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace gra {

inline int ResolveThreads(int requested) {
    if (requested > 0) {
        return requested;
    }
    return std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
}

template <class F>
void ParallelShards(int num_shards, int num_threads, F &&body) {
    if (num_shards <= 0) {
        return;
    }
    if (num_threads == 1) {
        for (int i = 0; i < num_shards; ++i) {
            body(i);
        }
        return;
    }
    std::atomic<int> cursor{0};
    std::vector<std::thread> threads;
    int spawn = std::min(num_threads, num_shards);
    threads.reserve(spawn);
    for (int t = 0; t < spawn; ++t) {
        threads.emplace_back([&] {
            int i = 0;
            while ((i = cursor.fetch_add(1)) < num_shards) {
                body(i);
            }
        });
    }
    for (auto &th : threads) {
        th.join();
    }
}

}  // namespace gra
