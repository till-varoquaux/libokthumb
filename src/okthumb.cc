#include "src/okthumb.h"
#include <string>
#include "src/image_reader.h"
#include "src/config.h"
#include "src/logging.h"
#include "src/resize.h"

ImgPipeline::ImgPipeline(const std::string& in, const Config& config)
        : reader_(img::get_reader(in, config.jpeg)), config_(&config) {}

const std::string& ImgPipeline::error() const {
    if (reader_) {
        return reader_->error();
    }
    static const std::string& failed_hdr =
            *new std::string("Failed to recognise the image type");
    return failed_hdr;
}

const std::string& ImgPipeline::warning() const {
    if (reader_) {
        return reader_->warning();
    }
    static const std::string& empty = *new std::string();
    return empty;
}

unsigned int ImgPipeline::src_height() const {
    return (reader_) ? reader_->src_height() : 0;
}

unsigned int ImgPipeline::src_width() const {
    return (reader_) ? reader_->src_width() : 0;
}

void ImgPipeline::crop(unsigned int left_x, unsigned int top_y,
                       unsigned int right_x, unsigned int bottom_y) {
    if (reader_) {
        reader_->dims.left_x = left_x;
        reader_->dims.top_y = top_y;
        reader_->dims.right_x = right_x;
        reader_->dims.bottom_y = bottom_y;
    }
}

void ImgPipeline::resize(unsigned int width, unsigned int height) {
    if (reader_) {
        reader_->dims.dst_width = width;
        reader_->dims.dst_height = height;
    }
}

// std::unique_ptr<image_reader> reader(img::get_reader(src, config.jpeg));
std::string ImgPipeline::run(file_type f) {
    if (!reader_) {
        return std::string();
    }

    auto img = reader_->decode();
    if (!reader_->ok()) {
        INFO_LOGGER << " error while reading image " << reader_->error()
                    << std::endl;
        return std::string();
    }

    if (reader_->warning().length() > 0) {
        INFO_LOGGER << " non fatal error while reading image: "
                    << reader_->warning() << std::endl;
    }

    if (img.width() != reader_->dims.dst_width ||
        img.height() != reader_->dims.dst_height) {
        img = ::resize(img, reader_->dims.dst_width, reader_->dims.dst_height,
                       scale_method(config_->resize.scale_method.val()));

        if (img.is_empty()) {
            INFO_LOGGER << " failed to resize image" << std::endl;
            return std::string();
        }
    }
    assert(img.width() == reader_->dims.dst_width &&
           img.height() == reader_->dims.dst_height);
    return std::string();
}
