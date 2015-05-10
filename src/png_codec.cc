#include "png_codec.h"

#include <iostream>
#include <sstream>
// #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
extern "C" {
#define PNG_DEBUG 3
#include <png.h>
}
#include <csetjmp>
#include <cassert>
#include "aligned_storage.h"
#include "logging.h"

// Sanity check of our forward declarations

static_assert(std::is_same<png_struct_def *, png_structp>::value,
              "improper forward type declaration.");

static_assert(std::is_same<png_info_struct *, png_infop>::value,
              "improper forward type declaration.");

#define PNGSIGSIZE 8

namespace {
inline unsigned char *string_ucptr(const std::string &str) {
  return reinterpret_cast<unsigned char *>(const_cast<char *>(str.data()));
}

void read_stringbuf(png_structp png_ptr, png_bytep data, png_size_t sz) {
  const std::streamsize requested = static_cast<std::streamsize>(sz);
  std::stringbuf *sbuf = reinterpret_cast<std::stringbuf *>(png_ptr->io_ptr);
  const std::streamsize bytesRead =
      sbuf->sgetn(reinterpret_cast<char *>(data), requested);
  if (requested != bytesRead) {
    ERR_LOGGER << "Input truncated" << std::endl;
    longjmp(png_jmpbuf(png_ptr), 1);
  }
}
}

bool is_png(const std::string &src) {
  if (src.size() < PNGSIGSIZE) {
    return false;
  }
  int is_png = 0;
  is_png = png_sig_cmp(string_ucptr(src), 0, PNGSIGSIZE);
  return (is_png == 0);
}

// Helper class to help us with the cleaning up.
png_reader::~png_reader() {
  if (!png_) {
    return;
  }
  auto infop = (info_ != nullptr) ? &info_ : nullptr;
  png_destroy_read_struct(&png_, infop, nullptr);
}

png_reader::png_reader(const std::string &src)
    : read_buf_(src, std::ios_base::in) {
  png_ =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png_) {
    set_error("Couldn't initialize png read struct");
    return;
  }

  info_ = png_create_info_struct(png_);
  if (!info_) {
    set_error("Couldn't initialize png info struct");
    return;
  }

  // Setup the error handling.
  if (setjmp(png_jmpbuf(png_)) != 0) {
    set_error("Failed to read header");
    return;
  }

  // Optionnaly Skip the magic number....
  //  png_set_sig_bytes(png, PNGSIGSIZE);

  png_set_read_fn(png_, static_cast<void *>(&read_buf_), read_stringbuf);

  // And read the signature,
  png_read_info(png_, info_);

  int bit_depth, color_type, interlace_type;
  png_uint_32 src_width, src_height;

  if (png_get_IHDR(png_, info_, &src_width, &src_height, &bit_depth,
                   &color_type, &interlace_type, nullptr, nullptr) != 1) {
    set_error("png reader: getIHDR failed");
    return;
  }

  width_ = static_cast<unsigned int>(src_width);
  height_ = static_cast<unsigned int>(src_height);

  // Set all the parameters to start reading in the image.
  bool has_alpha = color_type & PNG_COLOR_MASK_ALPHA;

  /* Convert paletted/grayscale to rgb */
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_);

  /*if the image has a transperancy set.. convert it to a full
    Alpha channel..*/
  if (png_get_valid(png_, info_, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_);
    has_alpha = true;
  }

  /* If the image is 16 bits per channel convert it down to 8. */
  if (bit_depth == 16) {
    png_set_strip_16(png_);
  }

  /* Expand each channel to 8 bits. */
  if (color_type == PNG_COLOR_TYPE_PALETTE ||
      (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)) {
    png_set_expand(png_);
  }

#if __BYTE_ORDER == __LITTLE_ENDIAN

  /* Add alpha channel if missing */
  if (!has_alpha) {
    png_set_add_alpha(png_, 0xFF, PNG_FILLER_AFTER);
  }

  png_set_bgr(png_);
#else
// if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
//  png_set_swap_alpha(png_);
#error "Unsupported endianness"
#endif
  /* Interlacing */
  if (interlace_type == PNG_INTERLACE_ADAM7)
    png_set_interlace_handling(png_);

  /* Commit add all those changes... */
  png_read_update_info(png_, info_);

  // Check that we've succefuly committed the changes
  // and are reading RGBA.
  if ((info_->color_type != PNG_COLOR_TYPE_RGBA) ||
      (png_get_rowbytes(png_, info_) != src_width * 4)) {
    set_error("Failed to pass the right magic to libpng to get proper ARGB");
    assert(info_->color_type == PNG_COLOR_TYPE_RGBA);
    assert(png_get_rowbytes(png_, info_) == src_width * 4);
    return;
  }
}

ARGBPixel *png_reader::src_line_buf(const dim_t) {
  if (src_line_buf_.is_empty())
    src_line_buf_.alloc(scaled_width());
  return src_line_buf_.ptr();
}

void png_reader::JunkLines(const dim_t dims) noexcept {
  if (dims.top_y == 0) {
    return;
  }
  unsigned char *scratch =
      reinterpret_cast<unsigned char *>(src_line_buf(dims));

  for (unsigned int i = 0; i < dims.top_y; i++) {
    png_read_row(png_, scratch, nullptr);
  }
}

void png_reader::ReadLines(const dim_t dims) noexcept {
  if (scaled_width() == dims.dst_width) {
    for (unsigned int i = 0; i < dims.dst_height; i++) {
      png_read_row(png_, reinterpret_cast<unsigned char *>(tmp_img_.row<0>(i)),
                   nullptr);
    }

  } else {
    ARGBPixel *scratch = src_line_buf(dims);

    for (unsigned int i = 0; i < dims.dst_height; i++) {
      png_read_row(png_, reinterpret_cast<unsigned char *>(scratch), nullptr);
      memcpy(tmp_img_.row<0>(i), scratch + dims.left_x,
             dims.dst_width * sizeof(ARGBPixel));
    }
  }
}

Image png_reader::decode_impl(const dim_t dims) {
  // Setup the error handling.
  if (setjmp(png_jmpbuf(png_)) != 0) {
    set_error("Failed to read image body.");
    return Image();
  }

  tmp_img_ = XRGBImage(dims.dst_width, dims.dst_height);

  JunkLines(dims);
  ReadLines(dims);
  return Image(std::move(tmp_img_));
}

//-----------------------------------------------------------------------------
//                                     PNG writter

class png_string_dest {
  std::string *outstring;
  size_t len;

public:
  png_string_dest(const png_string_dest &) = delete;
  png_string_dest &operator=(const png_string_dest &) = delete;
  ~png_string_dest() = default;
  explicit png_string_dest(std::string &dest);
  static void write_data(png_structp png_ptr, png_bytep data,
                         png_size_t length);
  static void flush(png_structp png_ptr);
};

png_string_dest::png_string_dest(std::string &dst) : outstring(&dst), len(0) {
  outstring->resize(4096);
}

void png_string_dest::write_data(png_structp png_ptr, png_bytep data,
                                 png_size_t length) {
  png_string_dest *dest = static_cast<png_string_dest *>(png_ptr->io_ptr);
  size_t slen = dest->outstring->length();
  size_t available = slen - dest->len;

  if (available < length) {
    dest->outstring->resize(std::max(2 * length, 2 * slen));
  }

  void *tgt = const_cast<char *>(dest->outstring->data() + dest->len);
  memcpy(tgt, data, length);
  dest->len += length;
}

void png_string_dest::flush(png_structp png_ptr) {
  png_string_dest *dest = static_cast<png_string_dest *>(png_ptr->io_ptr);
  dest->outstring->resize(dest->len);
}

class png_cleaner {
public:
  png_structp png = nullptr;
  png_infop info = nullptr;
  png_bytep row = nullptr;
  ~png_cleaner() {
    if (row != nullptr)
      free(row);
    if (png == nullptr)
      return;
    auto infop = (info != nullptr) ? &info : nullptr;
    png_destroy_write_struct(&png, infop);
  }
};

// TODO: Fixme
// http://www.libpng.org/pub/png/libpng-1.2.5-manual.html#section-3.7
std::string encode_png(const XRGBImage &src) {
  png_cleaner state;
  std::string retval;
  png_string_dest dest(retval);

  // Initialize write structure
  state.png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  state.info = png_create_info_struct(state.png);

  if (state.png == nullptr || state.info == nullptr) {
    ERR_LOGGER << "Error during png writer initialisation." << std::endl;
    return std::string();
  };

  if (setjmp(png_jmpbuf(state.png))) {
    ERR_LOGGER << "Error during png creation" << std::endl;
    return std::string();
  }

  png_set_write_fn(state.png, &dest, png_string_dest::write_data,
                   png_string_dest::flush);

  png_set_IHDR(state.png, state.info, src.width(), src.height(), 8,
               PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(state.png, state.info);

  for (unsigned int y = 0; y < src.height(); y++) {
    png_bytep row_pointers =
        reinterpret_cast<png_bytep>(const_cast<ARGBPixel *>(src.row<0>(y)));
    png_write_rows(state.png, &row_pointers, 1);
  }

  png_write_end(state.png, nullptr);

  return retval;
}
