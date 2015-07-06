// Test images
// http://www.imagecompression.info/test_images/

// TODO(till): see whether we can skip part of the image before decoding it by
// looking at how crop is implemented:
// http://svn.icmb.utexas.edu/svn/repository/trunk/zpub/sdkpub/freeimage/
// Source/FreeImageToolkit/JPEGTransform.cpp

// Jpeg explained
// https://www.cs.auckland.ac.nz/courses/compsci708s1c/lectures/jpeg_mpeg/jpeg.html

#include "logging.h"
#include "image.h"
#include "jpeg_codec.h"
#include "aligned_storage.h"

extern "C" {
#include <string.h>
#include <externals/libjpeg-turbo/jpegint.h>
}

#include <csetjmp>
#include <algorithm>
#include <string>

using tuple_helper::make_tuple_indices;
using tuple_helper::tuple_indices;

bool is_jpeg(const std::string &str) {
  const unsigned char *data =
      reinterpret_cast<const unsigned char *>(str.data());
  const size_t len = str.length();
  if (len < 8)
    return false;
  if (memcmp(data, "JFIF", 5) == 0) {
    return true;
  }
  if (memcmp(data, "Exif", 5) == 0) {
    return true;
  }
  if (data[0] == 0xFF && data[1] == 0xD8 && data[len - 2] == 0xFF &&
      data[len - 1] == 0xD9) {
    return true;
  }
  return false;
}

// Access the underlying buffer of an std string for reading or writing
// from C. As you can probably tell by the nasty cast going on here this
// not quite kosher.
static unsigned char *string_ucptr(const std::string &str) {
  return reinterpret_cast<unsigned char *>(const_cast<char *>(str.data()));
}

__attribute__((noreturn)) void jpeg_reader::fatal_error(j_common_ptr cinfo) {
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  jpeg_reader *reader = static_cast<jpeg_reader *>(cinfo->err);

  /* Always display the message. */
  char buf[JMSG_LENGTH_MAX];
  cinfo->err->format_message(cinfo, buf);

  reader->set_error(std::string(buf));

  /* Return control to the setjmp point */
  longjmp(reader->jmp_buffer_, 1);
}

void jpeg_reader::non_fatal_error(j_common_ptr cinfo) {
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  jpeg_reader *reader = static_cast<jpeg_reader *>(cinfo->err);

  /* Always display the message. */
  char buf[JMSG_LENGTH_MAX];
  cinfo->err->format_message(cinfo, buf);

  reader->set_warning(std::string(buf));
  return;
}

jpeg_reader::jpeg_reader(const std::string &src, const config::jpeg &config) {
  /* here we set up the standard libjpeg error handler */
  dinfo_.err = jpeg_std_error(this);
  // cinfo.err->trace_level = 3;
  error_exit = fatal_error;

  output_message = non_fatal_error;

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jmp_buffer_)) {
    return;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
  jpeg_create_decompress(&dinfo_);
#pragma clang diagnostic pop

  /* set */
  jpeg_mem_src(&dinfo_, string_ucptr(src), src.size());

  /* reading the image header which contains image information */
  jpeg_read_header(&dinfo_, true);

  dinfo_.dct_method = J_DCT_METHOD(config.dct_method.val());
  dinfo_.dither_mode = J_DITHER_MODE(config.dither_mode.val());
  dinfo_.do_fancy_upsampling = TRUE;
  dinfo_.scale_num = 1;
  dinfo_.scale_denom = 1;
  jpeg_calc_output_dimensions(&dinfo_);
}

void jpeg_reader::set_scale(unsigned int wanted) {
  if (wanted >= 8) {
    dinfo_.scale_denom = 8;
  } else if (wanted >= 4) {
    dinfo_.scale_denom = 4;
  } else if (wanted >= 2) {
    dinfo_.scale_denom = 2;
  }
  jpeg_calc_output_dimensions(&dinfo_);
}

unsigned int jpeg_reader::get_scale() const { return dinfo_.scale_denom; }

jpeg_reader::~jpeg_reader() { jpeg_destroy_decompress(&dinfo_); }

namespace {

enum src_colorspace { GRAYSCALE, YUV420, YUV444, RGB, UNKNOWN };

static const char *jcs_colorspace_to_string(J_COLOR_SPACE cs) {
  switch (cs) {
  case JCS_UNKNOWN:
    return "Unknown";
  case JCS_GRAYSCALE:
    return "Grayscale";
  case JCS_RGB:
    return "Rgb";
  case JCS_YCbCr:
    return "YCbCr";
  case JCS_CMYK:
    return "CYMK";
  case JCS_YCCK:
    return "YCCK";
  case JCS_RGB565:
    return "RGB 565";
  case JCS_EXT_RGB:
  case JCS_EXT_RGBX:
  case JCS_EXT_BGR:
  case JCS_EXT_BGRX:
  case JCS_EXT_XBGR:
  case JCS_EXT_XRGB:
  case JCS_EXT_RGBA:
  case JCS_EXT_BGRA:
  case JCS_EXT_ABGR:
  case JCS_EXT_ARGB:
    return "JCS_EXT";
  }
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
// #define PACKED_ARGB JCS_EXT_BGRA
#define PACKED_XRGB JCS_EXT_BGRX
#elif __BYTE_ORDER == __BIG_ENDIAN
// #define PACKED_ARGB JCS_EXT_ARGB
#define PACKED_XRGB JCS_EXT_XRGB
#else
#error "Unsupported endianness"
#endif

template <typename _IMG, size_t _IDX> struct encdec_helper_plane_t {
  typedef typename _IMG::template dim<_IDX> plane_t;

  static constexpr int h_samp = _IMG::max_h_samp / plane_t::h;
  static constexpr int v_samp = _IMG::max_v_samp / plane_t::v;

  static constexpr size_t height = h_samp * DCTSIZE;

  JSAMPROW row[height];

  encdec_helper_plane_t &fill_rows(_IMG *img, size_t offset) {
    offset /= plane_t::h;
    uint8_t *start = img->template data<_IDX>();
    const size_t stride = img->template stride<_IDX>();
    const size_t src_height = img->template height<_IDX>();
    for (uint i = 0; i < height; ++i) {
      const size_t src_row = std::min(src_height - 1, offset + i);
      row[i] = start + src_row * stride;
    }
    return *this;
  }

  static jpeg_component_info *fill_plane_info(jpeg_component_info *comp) {
    comp->h_samp_factor = h_samp;
    comp->v_samp_factor = v_samp;
    return comp;
  }

  encdec_helper_plane_t &copy_row(const _IMG &src, _IMG *dst,
                                  const size_t src_row, const size_t src_offset,
                                  const size_t dst_row) {
    if (dst_row % plane_t::h != 0) {
      return *this;
    }
    memcpy(dst->template data<_IDX>() +
               ((dst->template stride<_IDX>() * dst_row) / plane_t::h),
           src.template data<_IDX>() +
               ((src.template stride<_IDX>() * src_row) / plane_t::h +
                src_offset / plane_t::v),
           dst->template width<_IDX>());
    return *this;
  }
};

template <typename _IMG, typename _TupleIndexes> struct encdec_helper_impl_t;

template <typename _IMG, size_t... _Indices>
struct encdec_helper_impl_t<_IMG, tuple_indices<_Indices...>>
    : encdec_helper_plane_t<_IMG, _Indices>... {
  static constexpr size_t mcu_height = _IMG::max_h_samp * DCTSIZE;
  static constexpr unsigned int mcu_width = _IMG::max_v_samp * DCTSIZE;
  static constexpr size_t height = _IMG::max_h_samp * DCTSIZE;

  JSAMPARRAY jimg[sizeof...(_Indices)];

  template <size_t _N> using plane_helper_t = encdec_helper_plane_t<_IMG, _N>;

  template <size_t _N> plane_helper_t<_N> &get() { return *this; }

  encdec_helper_impl_t() : jimg{get<_Indices>().row...} {}

  static bool matches_subsampling(const struct jpeg_decompress_struct &dinfo) {
    return dinfo.num_components == _IMG::num_planes &&
           tuple_helper::all(dinfo.comp_info[_Indices].h_samp_factor ==
                                 plane_helper_t<_Indices>::h_samp...,
                             dinfo.comp_info[_Indices].v_samp_factor ==
                                 plane_helper_t<_Indices>::v_samp...);
  }

  void fill_rows(_IMG *img, size_t offset) {
    tuple_helper::swallow(get<_Indices>().fill_rows(img, offset)...);
  }

  static void fill_plane_info(struct jpeg_compress_struct *out) {
    tuple_helper::swallow(plane_helper_t<_Indices>::fill_plane_info(
        &out->comp_info[_Indices])...);
    out->input_components = _IMG::num_planes;
  }

  void copy_row(const _IMG &src, _IMG *dst, const size_t src_row,
                const size_t src_offset, const size_t dst_row) {
    // Parameter pack expansion needs to happen in a function call...
    tuple_helper::swallow(
        get<_Indices>().copy_row(src, dst, src_row, src_offset, dst_row)...);
  }
};

template <typename _IMG>
using encdec_helper_t =
    encdec_helper_impl_t<_IMG,
                         typename make_tuple_indices<_IMG::num_planes>::type>;

// A helper class used to print out the sumbsampling info of a
// jpeg_decompress_struct
struct subsamp_printer {
  const struct jpeg_decompress_struct &dinfo_;
  subsamp_printer(const jpeg_decompress_struct &dinfo) : dinfo_(dinfo) {}

  friend std::ostream &operator<<(std::ostream &stream,
                                  const subsamp_printer &s) {
    auto &dinfo = s.dinfo_;
    for (int i = 0; i < dinfo.num_components; ++i) {
      if (i > 0) {
        stream << ", ";
      }
      stream << dinfo.comp_info[i].h_samp_factor << ":"
             << dinfo.comp_info[i].v_samp_factor;
    }
    return stream;
  }
};

src_colorspace colorspace(const struct jpeg_decompress_struct &dinfo) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
  switch (dinfo.jpeg_color_space) {
  case JCS_YCbCr:
    if (encdec_helper_t<Yuv420Image>::matches_subsampling(dinfo))
      return YUV420;
    if (encdec_helper_t<Yuv444Image>::matches_subsampling(dinfo)) {
      INFO_LOGGER << "YUV444" << std::endl;
      return YUV444;
    }

    INFO_LOGGER << "YcbCr encoding with non 420 subsampling: "
                << subsamp_printer(dinfo) << std::endl;
    return UNKNOWN;
  case JCS_GRAYSCALE:
    return GRAYSCALE;
  case JCS_RGB:
    return RGB;
  default:
    INFO_LOGGER << "Colorspace: "
                << jcs_colorspace_to_string(dinfo.jpeg_color_space)
                << std::endl;
    return UNKNOWN;
  }
#pragma clang diagnostic pop
}
} // Namespace

////////////////////////////////////////////////////////////////////////////////
// Planar decoding

// TODO: make a direct version of that function for when the
// scaled_width == dst_width...
template <typename _IMG> Image jpeg_reader::planar_decode(dim_t const dims) {
  _IMG res(dims.dst_width, dims.dst_height);
  dinfo_.raw_data_out = true;

  const int scaled_dctsize =
      static_cast<int>(DCTSIZE * dinfo_.scale_num / dinfo_.scale_denom);

  encdec_helper_t<_IMG> dh;
  _IMG scratchbuff(scaled_width(), dh.height);
  dh.fill_rows(&scratchbuff, 0);
  if (setjmp(jmp_buffer_)) {
    return Image();
  }

  jpeg_start_decompress(&dinfo_);

  const int mcu_base =
      static_cast<int>(dh.mcu_width * dinfo_.scale_num / dinfo_.scale_denom);

  while (dinfo_.output_scanline < dims.bottom_y) {
    // This magic is taken from turbo jpeg to allow scaling
    // It might be specific to i420 scaling...
    for (unsigned int i = 0; i < _IMG::num_planes; ++i) {
      jpeg_component_info *compptr = &(dinfo_.comp_info[i]);
      /* When 4:2:0 subsampling is used with IDCT scaling, libjpeg will
         try
         to be clever and use the IDCT to perform upsampling on the U and
         V
         planes.  For instance, if the output image is to be scaled by 1/2
         relative to the JPEG image, then the scaling factor and
         upsampling
         effectively cancel each other, so a normal 8x8 IDCT can be used.
         However, this is not desirable when using the decompress-to-YUV
         functionality in TurboJPEG, since we want to output the U and V
         planes in their subsampled form.  Thus, we have to override some
         internal libjpeg parameters to force it to use the "scaled" IDCT
         functions on the U and V planes. */
      compptr->DCT_h_scaled_size = scaled_dctsize;

      compptr->MCU_sample_width =
          mcu_base * compptr->v_samp_factor / dinfo_.max_v_samp_factor;

      // dinfo_.idct->inverse_DCT[i] = dinfo_.idct->inverse_DCT[1];
    }

    const size_t orig = dinfo_.output_scanline;
    const size_t rows_read = jpeg_read_raw_data(&dinfo_, dh.jimg, dh.height);
    if (dinfo_.output_scanline < dims.top_y) {
      continue;
    }
    const size_t sz = std::min(dims.bottom_y - orig, rows_read);
    for (size_t i = 0; i < sz; i++) {
      const size_t src_row = orig + i;
      if (src_row < dims.top_y) {
        continue;
      }
      const size_t dst_row = src_row - dims.top_y;
      dh.copy_row(scratchbuff, &res, i, dims.left_x, dst_row);
    }
  }
  jpeg_abort_decompress(&dinfo_);
  return Image(std::move(res));
}

////////////////////////////////////////////////////////////////////////////////

Image jpeg_reader::fancy_decode(const dim_t dims) {
  Image res;

  dinfo_.out_color_space = PACKED_XRGB;
  dinfo_.output_components = 4;
  res = XRGBImage(dims.dst_width, dims.dst_height);
  XRGBImage *rgbx = res.val<ImageType::XRGB>();

  Aligned<unsigned char> src_line_buf(
      scaled_width() * static_cast<uint>(dinfo_.output_components));

  JSAMPROW outbuf[1] = {src_line_buf.ptr()};

  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jmp_buffer_)) {
    return Image();
  }
  // No allocs after this... RAII could be broken by longjmp
  jpeg_start_decompress(&dinfo_);

  // Junk out the lines we don't care about.
  while (dinfo_.output_scanline < dims.top_y) {
    // SKIA and firefox only read lines one by one so we won't bother doing
    // any
    // better for now...
    jpeg_read_scanlines(&dinfo_, outbuf, 1);
  }

  const unsigned char *orig =
      src_line_buf.ptr() +
      dims.left_x * static_cast<uint>(dinfo_.output_components);
  while (dinfo_.output_scanline < dims.bottom_y) {
    const unsigned int idx = dinfo_.output_scanline - dims.top_y;
    jpeg_read_scanlines(&dinfo_, outbuf, 1);
    memcpy(rgbx->row<0>(idx), orig, dims.dst_width * sizeof(ARGBPixel));
  }

  /* Clean it all up */
  jpeg_abort_decompress(&dinfo_);

  /* yup, we succeeded! */
  return res;
}

Image jpeg_reader::decode_impl(const dim_t dims) {
  switch (colorspace(dinfo_)) {
  case YUV420:
    return planar_decode<Yuv420Image>(dims);
  case GRAYSCALE:
    return planar_decode<GrayscaleImage>(dims);
  case YUV444:
    return planar_decode<Yuv444Image>(dims);
  case RGB:
  case UNKNOWN:
    return fancy_decode(dims);
  }
}

/** Writting JPEGs to string.... */

class jpeg_string_dest : public jpeg_destination_mgr {
  std::string *outstring; // The outstring's data starts at a proper
                          // allignement for JOCTET (it's unsigned charx).
public:
  jpeg_string_dest(const jpeg_string_dest &) = delete;
  jpeg_string_dest &operator=(const jpeg_string_dest &) = delete;
  ~jpeg_string_dest() = default;
  explicit jpeg_string_dest(std::string &dest);
  static void InitDestination(j_compress_ptr cinfo);
  static boolean EmptyOutputBuffer(j_compress_ptr cinfo);
  static void TermDestination(j_compress_ptr cinfo);
};

jpeg_string_dest::jpeg_string_dest(std::string &dst) : outstring(&dst) {
  init_destination = &jpeg_string_dest::InitDestination;
  empty_output_buffer = &jpeg_string_dest::EmptyOutputBuffer;
  term_destination = &jpeg_string_dest::TermDestination;
}

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */
void jpeg_string_dest::InitDestination(j_compress_ptr cinfo) {
  jpeg_string_dest *dest = static_cast<jpeg_string_dest *>(cinfo->dest);

  /* We could find a smarter start size for the outstring by looking at the
     image. */
  dest->outstring->resize(4096 * sizeof(JOCTET));

  dest->next_output_byte =
      reinterpret_cast<JOCTET *>(const_cast<char *>(dest->outstring->data()));
  dest->free_in_buffer = dest->outstring->size() / sizeof(JOCTET);
}

/*
 * Empty the output buffer --- called whenever buffer fills up.
 */
boolean jpeg_string_dest::EmptyOutputBuffer(j_compress_ptr cinfo) {
  jpeg_string_dest *dest = static_cast<jpeg_string_dest *>(cinfo->dest);

  const size_t curr_sz = dest->outstring->size();
  dest->outstring->resize(curr_sz * 2);
  // This might not need changing
  dest->next_output_byte = reinterpret_cast<JOCTET *>(
      const_cast<char *>(dest->outstring->data() + curr_sz));
  dest->free_in_buffer = curr_sz / sizeof(JOCTET);

  return 1;
}

/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 */
void jpeg_string_dest::TermDestination(j_compress_ptr cinfo) {
  jpeg_string_dest *dest = static_cast<jpeg_string_dest *>(cinfo->dest);

  dest->outstring->resize(dest->outstring->size() -
                          dest->free_in_buffer * sizeof(JOCTET));
}

namespace {

template <J_COLOR_SPACE _COLOR_SPACE, ImageType _Ty, typename _PIXEL_TYPE,
          typename... _PLANE_DIMS>
std::string
encode_planar(const PlanarImage<_Ty, _PIXEL_TYPE, _PLANE_DIMS...> &img,
              const config::jpeg &config) {
  typedef PlanarImage<_Ty, _PIXEL_TYPE, _PLANE_DIMS...> img_t;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  std::string res;
  class jpeg_string_dest destmgr(res);

  cinfo.err = jpeg_std_error(&jerr);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
  jpeg_create_compress(&cinfo);
#pragma clang diagnostic pop

  /* Set the output_mgr */
  cinfo.dest = &destmgr;

  /* Setting the parameters of the output file here */
  cinfo.image_width = img.width();
  cinfo.image_height = img.height();
  cinfo.input_components = img_t::num_planes;
  jpeg_set_defaults(&cinfo);

  cinfo.raw_data_in = true;

  jpeg_set_colorspace(&cinfo, _COLOR_SPACE);

  encdec_helper_t<img_t> dh;

  dh.fill_plane_info(&cinfo);

  cinfo.dct_method = J_DCT_METHOD(config.dct_method.val());
  cinfo.smoothing_factor = config.smoothing_factor;
  cinfo.optimize_coding = config.optimize_coding;

  // Pixel density?
  jpeg_set_quality(&cinfo, config.quality, true);

  /* Now do the compression .. */
  jpeg_start_compress(&cinfo, true);

  img_t &rimg = const_cast<img_t &>(img);
  size_t pos = 0;
  const size_t height = img.height();

  while (pos < height) {
    dh.fill_rows(&rimg, pos);
    pos += jpeg_write_raw_data(&cinfo, dh.jimg, dh.height);
  }

  /* cleanup */
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  return res;
}

} // namespace

std::string encode_jpeg(const Image &img, const config::jpeg &config) {
  if (img.type() == ImageType::GRAYSCALE) {
    return encode_planar<JCS_GRAYSCALE>(*img.val<ImageType::GRAYSCALE>(),
                                        config);
  } else {
    return encode_planar<JCS_YCbCr>(*to_yuv420(img), config);
  }
}
