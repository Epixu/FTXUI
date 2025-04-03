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

/// @brief Access a cell (Pixel) at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
const Pixel& Image::PixelAt(int x, int y) const {
  return stencil.Contain(x, y) ? pixels_[x + y*dimx_] : dev_null_pixel();
}

/// @brief Clear all the pixels from the screen
void Image::Clear() {
  for (auto& pixel : pixels_)
    pixel.reset_fully();
}

}  // namespace ftxui
