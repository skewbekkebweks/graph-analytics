#pragma once

#include <string>

namespace gra {

struct LeaderRankOptions {
    std::string input_dir;
    std::string output_path;
    int num_threads = 0;
    int max_iterations = 1000;
    double tolerance = 1e-6;
};

int RunLeaderRank(const LeaderRankOptions &opts);

}  // namespace gra
