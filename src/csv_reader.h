#pragma once

#include "common.h"

#include <fstream>

namespace gra {

class CsvEdgeReader {
public:
    explicit CsvEdgeReader(const std::string &path);

    bool IsOpen() const;
    bool Next(Vertex &from, Vertex &to);

private:
    std::ifstream in_;
};

}  // namespace gra
