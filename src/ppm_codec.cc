#include "ppm_codec.h"
#include <string>
#include <sstream>

extern "C" {
#include <arpa/inet.h>
}

// http://paulbourke.net/dataformats/ppm/
std::string encode_ppm(const XRGBImage &img) {
  std::stringstream sstr(std::stringstream::out);
  sstr << "P6 " << img.width() << " " << img.height() << " 255\n";
  for (unsigned int i = 0; i < img.height(); i++) {
    const ARGBPixel *row = img.row<0>(i);
    for (size_t j = 0; j < img.width(); ++j) {
      const ARGBPixel *pix = row + j;
      sstr << pix->r << pix->g << pix->b;
    }
  }
  // sstr << std::endl;
  return sstr.str();
}

std::string encode_ppm(const Image &img) { return encode_ppm(*to_xrgb(img)); }
