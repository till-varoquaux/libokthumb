#include "image.h"
#include "gif_codec.h"
#include <sstream>
#include <iostream>

/**
 * This codec works by using giflib's slurp; it is both slow AND wrong.
 * HOWEVER For most gifs this should lead to correct results (although it will
 * still be slow).
 * Files that do fancy tricks like drawing several images without any delay
 * in between them will not render properly
 *
 * https://en.wikipedia.org/wiki/Graphics_Interchange_Format#File_format
 *
 * The proper way to render the first frame to draw would be to get all the
 * IMAGE_DESC_RECORD_TYPE
 * and raster them until we hit we hit an extension with a delay.
 */
extern "C" {
#include <externals/giflib/lib/gif_lib.h>
}

static const char *str_gif_error(int ErrorCode) {
    const char *err = GifErrorString(ErrorCode);
    if (err == nullptr) {
        return "unknown error";
    } else {
        return err;
    }
}

/* Used to define a gif source that reads from a string.. */
static int read_stringbuf(GifFileType *t, GifByteType *mem, int len) {
    std::stringbuf *sbuf = reinterpret_cast<std::stringbuf *>(t->UserData);
    const std::streamsize bytesRead =
        sbuf->sgetn(reinterpret_cast<char *>(mem), len);

    return static_cast<int>(bytesRead);
}

bool is_gif(const std::string &src) {
    if (src.length() < 5) {
        return false;
    }
    return src[0] == 'G' && src[1] == 'I' && src[2] == 'F' && src[3] == '8' &&
        (src[4] == '9' || src[4] == '7') && src[5] == 'a';
}

gif_reader::~gif_reader() {
    if (gif != nullptr) {
        (void)DGifCloseFile(gif, nullptr);
        gif = nullptr;
    }
}

gif_reader::gif_reader(const std::string &data) : sbuf(data) {
  int Error;

  gif = DGifOpen(&sbuf, &read_stringbuf, &Error);

  if (gif == nullptr) {
    set_error(str_gif_error(Error));
    return;
  }

  if (DGifSlurp(gif) == GIF_ERROR) {
    set_error(str_gif_error(gif->Error));
    return;
  }

  if (gif->SavedImages == nullptr) {
    set_error("No image found in gif");
    return;
  }
}

unsigned int gif_reader::src_width() const {
  if (!ok()) {
    return 0;
  }
  const SavedImage &gif_img = gif->SavedImages[0];
  return static_cast<unsigned int>(gif_img.ImageDesc.Width);
}

unsigned int gif_reader::src_height() const {
  if (!ok()) {
    return 0;
  }
  const SavedImage &gif_img = gif->SavedImages[0];
  return static_cast<unsigned int>(gif_img.ImageDesc.Height);
}

Image gif_reader::decode_impl(dim_t dims) {
  const SavedImage &gif_img = gif->SavedImages[0];

  /* Lets dump it - set the global variables required and do it: */
  const ColorMapObject *ColorMap =
      (gif_img.ImageDesc.ColorMap ? gif_img.ImageDesc.ColorMap
                                  : gif->SColorMap);
  if (ColorMap == nullptr) {
    set_error("Gif Image does not have a colormap");
    return Image();
  }

  XRGBImage res(dims.dst_width, dims.dst_height);

  // This would probably be faster if we grabbed whole lines at once.
  for (unsigned int i = 0; i < dims.dst_height; i++) {
    ARGBPixel *row = res.row<0>(i);
    const GifByteType *src =
        &gif_img.RasterBits[(dims.top_y + i) * scaled_width() + dims.left_x];
    for (unsigned int j = 0; j < dims.dst_width; j++) {
      auto ColorMapEntry = &ColorMap->Colors[src[j]];
      // We are not bothering to check whether this is the transparent
      // color.
      row[j].a = 255;
      row[j].r = ColorMapEntry->Red;
      row[j].g = ColorMapEntry->Green;
      row[j].b = ColorMapEntry->Blue;
    }
  }

  int err_code;
  const int gif_res  = DGifCloseFile(gif, &err_code);
  gif = nullptr;
  if (gif_res != GIF_OK) {
    set_error(str_gif_error(err_code));
    return Image();
  }

  return Image(std::move(res));
}
