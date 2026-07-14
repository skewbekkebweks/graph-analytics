#pragma once

#include <string>

namespace gra {

struct PreprocessOptions {
    std::string input_path;
    std::string output_dir;
    int num_shards = 64;
};

int RunPreprocess(const PreprocessOptions &opts);

}  // namespace gra
