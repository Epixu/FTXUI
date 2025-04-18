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
  explicit Image(const Image&);
  explicit Image(Image&&);

  Image& operator = (const Image&);
  Image& operator = (Image&&);

  // Access a cell (Pixel) in the grid at a given position
  FTXUI_FORCE_INLINE()
  auto& PixelAt(int x, int y) {
    return pixels_[x + y * dimx_];
  }

  FTXUI_FORCE_INLINE()
  auto& PixelAt(int x, int y) const {
    return pixels_[x + y * dimx_];
  }

  // Get screen dimensions
  auto width () const { return dimx_; }
  auto height() const { return dimy_; }

  // Fill the image with space and default style
  void Clear();

  Box stencil;

  auto& get_pool  () const { return pool_;   }
  auto& get_pool  ()       { return pool_;   }
  auto& get_pixels() const { return pixels_; }
  auto& get_pixels()       { return pixels_; }

 protected:
  int dimx_;
  int dimy_;
  
  // acts as a pool, interfaced by pixels_. stuff is generally just appended on the
  // back, until you run compactify(), which passes through all pixels and
  // discards any bytes in this string, that aren't used anymore - it should happen
  // anyway, if memory behind the string moves on reallocation. this shouldn't really
  // happen too often - only if you push a really long grapheme (>4 bytes). it is much more efficient to reallocate this buffer, 
  // instead of allocate a new std::string per pixel every time something changes
  PackedString::Pool pool_;

  // No need of vector of vectors - these are just excess allocations
  // just index linearly: i = x + y*width
  std::vector<Pixel> pixels_;
};

}  // namespace ftxui

#endif  // FTXUI_SCREEN_IMAGE_HPP
