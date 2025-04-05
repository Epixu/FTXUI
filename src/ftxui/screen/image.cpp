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
      // Let's be generous and assume that every char is a 32bit unicode codepoint
      characters_.resize(dimx_ * dimy_ * 4, ' ');
      pixels_.reserve(dimx_ * dimy_);

      for (int i = 0; i < dimx_ * dimy_; ++i) {
         pixels_.emplace_back(
            std::string_view(characters_.data() + i * 4, 1)
         );
      }
   }
}

/// @brief Copy the contents of an image, pixel by pixel
Image::Image(const Image& rhs)
    : stencil{rhs.stencil},
      dimx_(rhs.dimx_),
      dimy_(rhs.dimy_),
      characters_(rhs.characters_) {
   if (dimx_ * dimy_) {
      pixels_.reserve(dimx_ * dimy_);

      for (auto& rhsp : rhs.pixels_) {
         pixels_.emplace_back(std::string_view(
            characters_.data() + (rhsp.get_grapheme().data() - rhs.characters_.data()),
            rhsp.get_grapheme().size()
         ));
      }
   }
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

/// @brief  Copy an image by reusing local memory if possible
Image& Image::operator = (const Image& rhs) {
   stencil = rhs.stencil;
   dimx_ = rhs.dimx_;
   dimy_ = rhs.dimy_;
   characters_ = rhs.characters_;

   pixels_.clear();
   pixels_.reserve(dimx_ * dimy_);

   for (auto& rhsp : rhs.pixels_) {
      pixels_.emplace_back(std::string_view(
         characters_.data() + (rhsp.get_grapheme().data() - rhs.characters_.data()),
         rhsp.get_grapheme().size()
      ));
   }

   return *this;
}

/// @brief  transfer ownership of an image
Image& Image::operator = (Image&& rhs) {
   stencil = rhs.stencil;
   dimx_ = rhs.dimx_;
   dimy_ = rhs.dimy_;
   characters_ = std::move(rhs.characters_);
   pixels_ = std::move(rhs.pixels_);
   rhs.stencil = {0, 0, 0, 0};
   rhs.dimx_ = rhs.dimy_ = 0;
   return *this;
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
/*Pixel& Image::PixelAt(int x, int y) {
  return stencil.Contain(x, y) ? pixels_[x + y*dimx_] : dev_null_pixel(); // this is very costly to do per-pixel. just check and min/max bounding boxes once before you iterate pixels
}*/

Pixel& Image::PixelAt(int x, int y) {
   if (not stencil.Contain(x, y))
      exit(-1); //TODO make sure to delete this branch when safety is ensured
  return pixels_[x + y*dimx_];
}

/// @brief Access a cell (Pixel) at a given position.
/// @param x The cell position along the x-axis.
/// @param y The cell position along the y-axis.
/*const Pixel& Image::PixelAt(int x, int y) const {
  return stencil.Contain(x, y) ? pixels_[x + y*dimx_] : dev_null_pixel();
}*/

const Pixel& Image::PixelAt(int x, int y) const {
   if (not stencil.Contain(x, y))
      exit(-1); //TODO make sure to delete this branch when safety is ensured
   return pixels_[x + y*dimx_];
}

/// @brief Clear all the pixels from the screen
void Image::Clear() {
  for (auto& pixel : pixels_)
    pixel.reset();
}

/// @brief Inserts data into the memory pool, and returns a non-owning view to it
/// @attention call this only if data is longer than 4 bytes to avoid performance issues
std::string_view Image::pool_chardata(const std::string_view& data) {
   auto old_ptr = characters_.data();
   auto old_siz = characters_.size();
   //auto old_cap = characters_.capacity();
   characters_ += data;
   if (old_ptr != characters_.data()) {
      //a reallocation happened and memory moved, so compactify, reinterface pixels etc...
      compactify(old_ptr);
      old_ptr = characters_.data();
      old_siz = characters_.size();

      characters_ += data;
      if (characters_.data() != old_ptr)
         exit(-1); //TODO just making sure that memory never moves here. if it moves, it will invalidate all previously inserted pixels_
   }

   return {characters_.data() + old_siz, characters_.size() - old_siz};
}

/// @attention when this function is called, you can assume that any pixel.get_grapheme().data() pointer is invalid
/// pixel.get_grapheme().size() however is still valid
void Image::compactify(const char* olddata) {
   // First pass calculates pool requirements, so that pool never reallocates while we're doing this
   size_t pool_requirement = 0;
   for (auto& p : pixels_)
      pool_requirement += p.get_grapheme().size();
   if (pool_requirement < dimx_ * dimy_ * 4)
      pool_requirement = dimx_ * dimy_ * 4;

   // Allocate a new pool with a more generous size
   std::string temp;
   temp.reserve(pool_requirement*2);
   const auto old_ptr = temp.data();

   // Remap all pixels
   for (auto& p : pixels_) {
      const auto old_siz = temp.size();
      temp += std::string_view(characters_.data() + (p.get_grapheme().data() - olddata), p.get_grapheme().size());
      if (temp.data() != old_ptr)
         exit(-1); //TODO just making sure that memory never moves here. if it moves, it will invalidate all previously inserted pixels_

      p.reuse_grapheme(std::string_view(temp.data() + old_siz, temp.size() - old_siz));
   }

   // Switch to the new pool
   characters_ = std::move(temp);
   if (characters_.data() != old_ptr)
      exit(-1); //TODO just making sure that memory never moves here. if it moves, it will invalidate all previously inserted pixels_
}


// I noticed, that at many places "character = " "; // Consider the pixel written."
// those occurences are replaced with this call
void Pixel::reset_grapheme() {
  character(0) = ' ';
  grapheme = std::string_view(grapheme.data(), 1);
}

void Pixel::set_grapheme(const std::string_view& g, Image& pixel_owner) {
  if (g.size() > 4) {
     grapheme = pixel_owner.pool_chardata(g);
  }
  else {
     memcpy(const_cast<char*>(grapheme.data()), g.data(), g.size());
     grapheme = std::string_view(grapheme.data(), g.size());
  }
}

void Pixel::reset() {
   PixelBase::operator = (PixelBase());
   reset_grapheme();
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
