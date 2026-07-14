#include "csv_reader.h"

#include <charconv>
#include <string>

namespace gra {

CsvEdgeReader::CsvEdgeReader(const std::string &path) : in_(path) {
}

bool CsvEdgeReader::IsOpen() const {
    return in_.is_open();
}

bool CsvEdgeReader::Next(Vertex &from, Vertex &to) {
    std::string line;
    while (std::getline(in_, line)) {
        const char *p = line.data();
        const char *end = p + line.size();

        auto parsed_from = std::from_chars(p, end, from);
        if (parsed_from.ec != std::errc()) {
            continue;
        }
        p = parsed_from.ptr;
        while (p < end && (*p == ',' || *p == '\t' || *p == ' ')) {
            ++p;
        }
        if (std::from_chars(p, end, to).ec == std::errc()) {
            return true;
        }
    }
    return false;
}

}  // namespace gra
