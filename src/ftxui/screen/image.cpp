// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <sstream>  // IWYU pragma: keep
#include <string>
#include <vector>

#include "ftxui/screen/image.hpp"
#include "ftxui/screen/pixel.hpp"

namespace ftxui {

namespace {
Pixel& dev_null_pixel() {
  static Pixel pixel;
  return pixel;
}
}  // namespace

/// @brief Create a new image with default pixels
Image::Image(int dimx, int dimy)
    : stencil{0, dimx - 1, 0, dimy - 1},
      dimx_(dimx),
      dimy_(dimy),
      pixels_(dimy * dimx) {
   // Let's be generous and assume that every char is a 32bit unicode codepoint
   // Double that for some more pool space
   characters_.reserve(dimy * dimx * 4 * 2);
   // First character is always ' ', for use with reset_grapheme
   characters_ += ' ';
   //TODO we can reserve entire charsets here and index them, instead of always appending
   // this way an append will happen only in the rare case of a grapheme actually being displayed
}

/// @brief Copy the contents of an image, pixel by pixel
Image::Image(const Image& rhs)
    : stencil{rhs.stencil},
      dimx_(rhs.dimx_),
      dimy_(rhs.dimy_),
      pixels_(rhs.dimx_* rhs.dimy_) {
   // Let's be generous and assume that every char is a 32bit unicode codepoint
   // Double that for some more pool space
   characters_.reserve(rhs.dimx_ * rhs.dimy_ * 4 * 2);
   // First character is always ' ', for use with reset_grapheme
   characters_ += ' ';

   // regenerate the pool locally
   for (int i = 0; i < rhs.dimx_ * rhs.dimy_; ++i)
      pixels_[i].copy_pixel_data(rhs.pixels_[i], *this);
}

/// @brief Transfer ownership from another image
Image::Image(Image&& rhs)
    : stencil{rhs.stencil},
      dimx_(rhs.dimx_),
      dimy_(rhs.dimy_),
      characters_(std::move(rhs.characters_)),
      pixels_(std::move(rhs.pixels_)) {
   // reset source
   rhs.stencil = {0, 0, 0, 0};
   rhs.dimx_ = rhs.dimy_ = 0;
}

/// @brief Access a character in a cell at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
/*std::string_view& Image::at(int x, int y) { // deprecated, because we aren't allowed to modify grapheme directly
  return PixelAt(x, y).character;             // you must now explicitly use PixelAt(x,y).set_grapheme instead
}*/

/// @brief Access a character in a cell at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
const std::string_view& Image::at(int x, int y) const {
  return PixelAt(x, y).get_grapheme();
}

/// @brief Access a cell (Pixel) at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
Pixel& Image::PixelAt(int x, int y) {
  return stencil.Contain(x, y) ? pixels_[x + y*dimx_] : dev_null_pixel();
}

Pixel& Image::PixelAtUnsafe(int x, int y) {
  return pixels_[x + y*dimx_];
}

/// @brief Access a cell (Pixel) at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
const Pixel& Image::PixelAt(int x, int y) const {
  return stencil.Contain(x, y) ? pixels_[x + y*dimx_] : dev_null_pixel();
}

const Pixel& Image::PixelAtUnsafe(int x, int y) const {
  return pixels_[x + y*dimx_];
}

/// @brief Clear all the pixels from the screen
void Image::Clear() {
  for (auto& pixel : pixels_)
    pixel.reset_fully();
}

}  // namespace ftxui
