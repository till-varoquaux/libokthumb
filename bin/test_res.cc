#include <fstream>
#include <cstdlib>
#include <iostream>
#include "src/config.h"
#include "src/okthumb.h"

// For testing purposes: read a whole file into a std::str
static bool read_local_file(const std::string &filename, std::string *tgt) {
    std::ifstream in(filename, std::ios::in);
    if (in) {
        in.seekg(0, std::ios::end);
        const ssize_t sz = in.tellg();
        tgt->resize(static_cast<size_t>(sz));
        in.seekg(0, std::ios::beg);
        in.read(const_cast<char *>(tgt->c_str()), sz);
        in.close();
        return true;
    }
    return false;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: decode FILE" << std::endl;
        return EXIT_FAILURE;
    }
    std::string src, res;
    Config cfg;
    bool fst = true;
    const uint32_t tgt_width = 400;

    for (int i = 0; i < 10; ++i) {
        if (!read_local_file(argv[1], &src)) {
            std::cerr << "Failed to read file" << std::endl;
            return EXIT_FAILURE;
        }
        ImgPipeline pipe(src, cfg);
        if (pipe.src_width() > pipe.src_height()) {
            auto extra = pipe.src_width() - pipe.src_height();
            pipe.crop(extra / 2, 0, pipe.src_height() + (extra / 2), 0);
        } else {
            auto extra = pipe.src_height() - pipe.src_width();
            pipe.crop(0, extra / 2, 0, pipe.src_width() + (extra / 2));
        }
        pipe.resize(tgt_width);
        res = pipe.run(file_type::JPEG);
    }
    std::cout << res << std::endl;
    return EXIT_SUCCESS;
}
