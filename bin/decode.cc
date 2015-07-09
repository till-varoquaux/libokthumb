#include "logging.h"
#include "resize.h"
#include "ppm_codec.h"
#include "image_reader.h"
#include <fstream>
#include <cstdlib>
#include "image.h"


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
    std::string src;

    if (!read_local_file(argv[1], &src)) {
        std::cerr << "Failed to read file" << std::endl;
        return EXIT_FAILURE;
    }

    auto reader = img::get_reader(src);

    if (!reader) {
        std::cerr << "Failed to read image" << std::endl;
        return EXIT_FAILURE;
    }

    if (!reader->ok()) {
        std::cerr << "failed to read the picture's header: " << reader->error()
                   << std::cerr;
        return EXIT_FAILURE;
    }

    Image img = reader->decode();

    if (!reader->ok()) {
        std::cerr << "error while reading image: " << reader->error()
                  << std::endl;
        return EXIT_FAILURE;
    }

    INFO_LOGGER << to_string(img.type()) << ":" << img.width() << "x"
                << img.height() << std::endl;

    std::cout << encode_ppm(img) << std::endl;
    return EXIT_SUCCESS;
}
