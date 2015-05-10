#include "logging.h"
#include "resize.h"
#include "ppm_codec.h"
#include "image_reader.h"
#include <fstream>

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
        FAIL("Usage: decode FILE");
    }
    std::string src;

    if (!read_local_file(argv[1], &src)) {
        FAIL("Failed to read file");
    }

    bool ok;
    unsigned int width = 0;
    unsigned int height = 0;

    auto reader = img::get_reader(src);

    if (!reader) {
        FAIL("Failed to read image");
    }

    if (!reader->ok()) {
        FAIL("failed to read the picture's header: " << reader->error());
    }

    width = reader->src_width();
    height = reader->src_height();

    Image img = reader->decode();

    if (!reader->ok()) {
        FAIL("error while reading image: " << reader->error());
    }

    INFO_LOGGER << to_string(img.type()) << ":" << img.width() << "x"
                << img.height() << std::endl;

    std::cout << encode_ppm(img) << std::endl;
    return 0;
}
