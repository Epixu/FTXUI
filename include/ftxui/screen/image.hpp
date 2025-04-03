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

  // Access a character in the grid at a given position.
  //std::string_view& at(int x, int y);
  const std::string_view& at(int x, int y) const;

  // Access a cell (Pixel) in the grid at a given position.
  Pixel& PixelAt(int x, int y);
  const Pixel& PixelAt(int x, int y) const;

  Pixel& PixelAtUnsafe(int x, int y);
  const Pixel& PixelAtUnsafe(int x, int y) const;

  // Get screen dimensions.
  int width() const { return dimx_; }
  int height() const { return dimy_; }

  // Fill the image with space and default style
  void Clear();

  Box stencil;

  auto& get_pool() const { return characters_; }

  std::string_view pool_chardata(const std::string_view& data) {
    const auto old_ptr = characters_.data();
    const auto old_siz = characters_.size();
    characters_ += data;
    if (old_ptr != characters_.data()) {
      //TODO a reallocation happened and memory moved, so compactify, reinterface pixels etc...
      exit(-1);
    }

    return {characters_.data() + old_siz, characters_.size() - old_siz};
  }

 protected:
  int dimx_;
  int dimy_;
  
  // acts as a pool, interfaced by pixels_. stuff is generally just appended on the
  // back, until you run compactify(), which passes through all pixels and
  // discards any bytes in this string, that aren't used anywhere - it should happen
  // anyway, if memory behind the string moves on reallocation. this shouldn't really
  // happen too often. maybe when the pool becomes over a megabyte or something?
  // it is much more efficient to reallocate this buffer, 
  // instead of allocate a new std::string per pixel every time something changes :(
  std::string characters_;
  void compactify();

  // No need of vector of vectors - these are just excess allocations
  // just index linearly: i = x + y*width
  std::vector<Pixel> pixels_;
};

// I noticed, that at many places "character = " "; // Consider the pixel written."
// happens, so I decided to reserve the first character in characters_ to be always
// 'space', so that we can very quickly reset a grapheme without any reallocation
//TODO now this can be taken a lot further - we can insert all charsets inside the image
// (or just the ones potentially used in an image)
// from the get go and use those as a palette of sorts, avoiding appending new
// data to the pool in 99% of the cases!!!!
void Pixel::reset_grapheme(Image& pixel_owner) {
  grapheme = {pixel_owner.get_pool().data(), 1};
}

void Pixel::set_grapheme(const std::string_view& g, Image& pixel_owner) {
  grapheme = pixel_owner.pool_chardata(g);
}

void Pixel::reset(Image& pixel_owner) {
  new (this) Pixel;
  reset_grapheme(pixel_owner);
}

void Pixel::copy_pixel_data(const PixelStandalone& rhs, Image& pixel_owner) {
   PixelBase::operator = (rhs);
   set_grapheme(rhs.get_grapheme(), pixel_owner);
}

void Pixel::copy_pixel_data(const Pixel& rhs, Image& pixel_owner) {
   PixelBase::operator = (rhs);
   set_grapheme(rhs.get_grapheme(), pixel_owner);
}


}  // namespace ftxui

#endif  // FTXUI_SCREEN_IMAGE_HPP
