// Copyright 2024 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef FTXUI_SCREEN_IMAGE_HPP
#define FTXUI_SCREEN_IMAGE_HPP

#include <string>  // for string, basic_string, allocator
#include <vector>  // for vector

#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/pixel.hpp"  // for Pixel

namespace ftxui {

/// @brief A rectangular grid of Pixel.
/// @ingroup screen
class Image {
 public:
  // Constructors:
  Image() = delete;
  Image(int dimx, int dimy);
  Image(const Image&);
  Image(Image&&);

  Image& operator = (const Image&);
  Image& operator = (Image&&);

  // Access a character in the grid at a given position.
  //std::string_view& at(int x, int y);
  const std::string_view& at(int x, int y) const;

  // Access a cell (Pixel) in the grid at a given position.
  Pixel& PixelAt(int x, int y);
  const Pixel& PixelAt(int x, int y) const;

  // Get screen dimensions.
  auto width() const { return dimx_; }
  auto height() const { return dimy_; }

  // Fill the image with space and default style
  void Clear();

  Box stencil;

  auto& get_pool() const { return characters_; }
  auto& get_pixels() { return pixels_; }

  std::string_view pool_chardata(const std::string_view&);

 protected:
  int dimx_;
  int dimy_;
  
  // acts as a pool, interfaced by pixels_. stuff is generally just appended on the
  // back, until you run compactify(), which passes through all pixels and
  // discards any bytes in this string, that aren't used anymore - it should happen
  // anyway, if memory behind the string moves on reallocation. this shouldn't really
  // happen too often - only if you push a really long grapheme (>4 bytes). it is much more efficient to reallocate this buffer, 
  // instead of allocate a new std::string per pixel every time something changes
  std::string characters_;
  void compactify(const char* oldptr);

  // No need of vector of vectors - these are just excess allocations
  // just index linearly: i = x + y*width
  std::vector<Pixel> pixels_;
};

}  // namespace ftxui

#endif  // FTXUI_SCREEN_IMAGE_HPP
