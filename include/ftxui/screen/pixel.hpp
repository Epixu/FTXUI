// Copyright 2024 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef FTXUI_SCREEN_PIXEL_HPP
#define FTXUI_SCREEN_PIXEL_HPP

#include <cstdint>                 // for uint8_t
#include <vector>
#include "ftxui/screen/color.hpp"  // for Color, Color::Default
#include "ftxui/screen/packed_string.hpp"  // for PackedString

namespace ftxui {

class Image;

struct PixelBase
{
   // A bit field representing the style:
   union {
      struct {
         bool blink : 1;
         bool bold : 1;
         bool dim : 1;
         bool italic : 1;
         bool inverted : 1;
         bool underlined : 1;
         bool underlined_double : 1;
         bool strikethrough : 1;
         bool automerge : 1;
      };

      uint16_t style = 0;
   };

   // The hyperlink associated with the pixel.
   // 0 is the default value, meaning no hyperlink.
   // It's an index for accessing Screen meta data
   uint8_t hyperlink = 0;

   // Colors:
   Color background_color = Color::Default;
   Color foreground_color = Color::Default;

   FTXUI_FORCE_INLINE()
   void reset_style() {
      style = 0;
      hyperlink = 0;
      background_color = Color::Default;
      foreground_color = Color::Default;
   }
};

/// @brief A Unicode character/grapheme and its associated style.
/// @detail This pixel is not embedded, and owns its character data 
///         (it's just the old Pixel implementation)
/// @ingroup screen
struct PixelStandalone : PixelBase {
  std::string grapheme;
};


/// @brief A Unicode character/grapheme and its associated style.
/// @detail This is an embedded pixel, the real character data is either packed if small
///         or contained in the Image producing the pixel
/// @ingroup screen
struct Pixel : PixelBase {
  // The graphemes stored into the pixel. To support combining characters,
  // like: a?, this can potentially contain multiple codepoints.
  // The pixel itself however doesn't have ownership over the characters to
  // avoid lots of small allocations, instead it only interfaces the main string
  // inside an image or uses small-value-optimization.
  PackedString grapheme;
};

}  // namespace ftxui

#endif  // FTXUI_SCREEN_PIXEL_HPP
