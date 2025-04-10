// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <sstream>  // IWYU pragma: keep
#include <string>
#include <vector>

#include "ftxui/screen/image.hpp"
#include "ftxui/screen/pixel.hpp"

namespace ftxui {

/// @brief Create a new image with default pixels
Image::Image(int dimx, int dimy)
    : stencil{0, dimx - 1, 0, dimy - 1},
      dimx_(dimx),
      dimy_(dimy) {
   if (dimx_ * dimy_) {
      //pool_.resize(dimx_ * dimy_ * 2, 0); // needed only when non-ascii, will be allocated on demand
      pixels_.resize(dimx_ * dimy_);
   }
}

/// @brief Deep-copy the contents of an image, pixel by pixel
Image::Image(const Image& rhs)
    : stencil{rhs.stencil},
      dimx_(rhs.dimx_),
      dimy_(rhs.dimy_),
      pool_(rhs.pool_),
      pixels_(rhs.pixels_) {}

/// @brief Transfer ownership from another image
Image::Image(Image&& rhs)
    : stencil{rhs.stencil},
      dimx_(rhs.dimx_),
      dimy_(rhs.dimy_),
      pool_(std::move(rhs.pool_)),
      pixels_(std::move(rhs.pixels_)) {
   // reset source
   rhs.stencil = {0, 0, 0, 0};
   rhs.dimx_ = rhs.dimy_ = 0;
}

/// @brief  Copy an image by reusing local memory if possible
Image& Image::operator = (const Image& rhs) {
   stencil = rhs.stencil;
   dimx_ = rhs.dimx_;
   dimy_ = rhs.dimy_;
   pool_ = rhs.pool_;
   pixels_ = rhs.pixels_;
   return *this;
}

/// @brief  transfer ownership of an image
Image& Image::operator = (Image&& rhs) {
   stencil = rhs.stencil;
   dimx_ = rhs.dimx_;
   dimy_ = rhs.dimy_;
   pool_ = std::move(rhs.pool_);
   pixels_ = std::move(rhs.pixels_);
   rhs.stencil = {0, 0, 0, 0};
   rhs.dimx_ = rhs.dimy_ = 0;
   return *this;
}

/// @brief Clear all the pixels from the screen
void Image::Clear() {
  for (auto& pixel : pixels_)
    pixel.grapheme = ' ';
}

}  // namespace ftxui
