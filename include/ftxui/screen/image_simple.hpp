// Copyright 2024 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#pragma once

#include <string>  // for string, basic_string, allocator
#include <vector>  // for vector

#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/color.hpp"  // for Color, Color::Default
#include "ftxui/screen/packed_string.hpp"  // for PackedString

namespace ftxui {

/// @brief A rectangular grid of Pixel.
/// @ingroup screen
class ImageSimple {
 public:
  // Constructors:
  ImageSimple() = default;
  explicit ImageSimple(const ImageSimple&) = default;
  explicit ImageSimple(ImageSimple&&) = default;

  ImageSimple(int dimx, int dimy)
     : dimx_(dimx)
     , dimy_(dimy) {
     characters_.resize(dimx * dimy, ' ');
     colors_.resize(dimx * dimy, Color::Default);
  }

  ImageSimple& operator = (const ImageSimple&) = default;
  ImageSimple& operator = (ImageSimple&&) = default;

  // Access a cell (Pixel) in the grid at a given position
  FTXUI_FORCE_INLINE()
  auto& CharAt(int x, int y) {
    return characters_[x + y * dimx_];
  }
  auto& ColorAt(int x, int y) {
    return colors_[x + y * dimx_];
  }

  FTXUI_FORCE_INLINE()
  auto& CharAt(int x, int y) const {
    return characters_[x + y * dimx_];
  }
  FTXUI_FORCE_INLINE()
  auto& ColorAt(int x, int y) const {
    return colors_[x + y * dimx_];
  }

  // Get screen dimensions
  auto width () const { return dimx_; }
  auto height() const { return dimy_; }

  auto& get_colors () const { return colors_;     }
  auto& get_colors ()       { return colors_;     }
  auto& get_chars  () const { return characters_; }
  auto& get_chars  ()       { return characters_; }

 protected:
  int dimx_ = 0;
  int dimy_ = 0;
  
  std::vector<char>  characters_;
  std::vector<Color> colors_;
};

}  // namespace ftxui