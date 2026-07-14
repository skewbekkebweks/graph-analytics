#include "leaderrank.h"
#include "preprocess.h"

#include <cstdlib>
#include <iostream>
#include <string>

void PrintUsage() {
    std::cout << "usage:\n"
                 "  gra preprocess --input <edges.csv> --output <dir> [--shards <N>]\n"
                 "  gra run --input <dir> --output <ranks.csv>\n"
                 "          [--threads <N>] [--max-iter <N>] [--tol <X>]\n"
                 "          defaults: --max-iter 1000  --tol 1e-6\n";
}

int CommandPreprocess(int argc, char **argv) {
    gra::PreprocessOptions opts;
    for (int i = 2; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            PrintUsage();
            return 1;
        }
        const char *value = argv[i + 1];
        if (arg == "--input") {
            opts.input_path = value;
        } else if (arg == "--output") {
            opts.output_dir = value;
        } else if (arg == "--shards") {
            opts.num_shards = std::atoi(value);
        } else {
            std::cerr << "error: unknown argument " << arg << "\n";
            PrintUsage();
            return 1;
        }
    }
    if (opts.input_path.empty() || opts.output_dir.empty()) {
        PrintUsage();
        return 1;
    }
    return gra::RunPreprocess(opts);
}

int CommandRun(int argc, char **argv) {
    gra::LeaderRankOptions opts;
    for (int i = 2; i < argc; i += 2) {
        std::string arg = argv[i];
        if (i + 1 >= argc) {
            PrintUsage();
            return 1;
        }
        const char *value = argv[i + 1];
        if (arg == "--input") {
            opts.input_dir = value;
        } else if (arg == "--output") {
            opts.output_path = value;
        } else if (arg == "--threads") {
            opts.num_threads = std::atoi(value);
        } else if (arg == "--max-iter") {
            opts.max_iterations = std::atoi(value);
        } else if (arg == "--tol") {
            opts.tolerance = std::atof(value);
        } else {
            std::cerr << "error: unknown argument " << arg << "\n";
            PrintUsage();
            return 1;
        }
    }
    if (opts.input_dir.empty() || opts.output_path.empty()) {
        PrintUsage();
        return 1;
    }
    return gra::RunLeaderRank(opts);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }
    std::string command = argv[1];
    if (command == "preprocess") {
        return CommandPreprocess(argc, argv);
    }
    if (command == "run") {
        return CommandRun(argc, argv);
    }
    PrintUsage();
    return 1;
}
