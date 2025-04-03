// Copyright 2024 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef FTXUI_SCREEN_PIXEL_HPP
#define FTXUI_SCREEN_PIXEL_HPP

#include <cstdint>                 // for uint8_t
#include <string_view>             // for string_view
#include "ftxui/screen/color.hpp"  // for Color, Color::Default

namespace ftxui {

class Image;

/// @brief A Unicode character/grapheme and its associated style.
/// @tparam OWNERSHIP - if true, the grapheme representation is std::string which allocates
///                   - if false, the grapheme is 'embedded' inside a pool and never allocates
/// @ingroup screen
template<bool OWNERSHIP>
struct TPixel {
  TPixel()
      : blink(false),
        bold(false),
        dim(false),
        italic(false),
        inverted(false),
        underlined(false),
        underlined_double(false),
        strikethrough(false),
        automerge(false) {}

  // One does not simply copy a pixel - graphemes are pooled and proper ownership must be maintained
  TPixel(const TPixel&) = delete;
  TPixel(TPixel&&) = delete;
  TPixel& operator=(const TPixel&) = delete;
  TPixel& operator=(TPixel&&) = delete;

  // A bit field representing the style:
  bool blink : 1;
  bool bold : 1;
  bool dim : 1;
  bool italic : 1;
  bool inverted : 1;
  bool underlined : 1;
  bool underlined_double : 1;
  bool strikethrough : 1;
  bool automerge : 1;

  // The hyperlink associated with the pixel.
  // 0 is the default value, meaning no hyperlink.
  // It's an index for accessing Screen meta data
  uint8_t hyperlink = 0;

  // The graphemes stored into the pixel. To support combining characters,
  // like: a?, this can potentially contain multiple codepoints.
  // The pixel itself however doesn't have ownership over the characters to avoid lots of small allocations,
  // instead it only interfaces the main string inside an image. std::string_view is probably not
  // the best choice, as it implied the view is immutable, but C++11 lacks std::span afaik, and I
  // don't really want to use raw pointers and counters. So I just const_cast string_view::data()
  // It shouldn't really matter in the grand scheme of things
  // @attention you should never assign a string literal to grapheme!!!!
  // That's why I have encapsulated it very tightly
  char& character(size_t n) {
     // this direct access is used across the code, so I assume it considers the proper limits at place of usage
     // i've simply routed it through the data pointer
     return const_cast<char*>(grapheme.data())[n];
  }

  // Makes sure data is inserted into Image::characters_, and just interfaced here
  void  reset(Image& pixel_owner);
  void  reset_fully() { new (this) Pixel; }
  void  reset_grapheme(Image& pixel_owner);
  void  set_grapheme(const std::string_view& g, Image& pixel_owner);
  auto& get_grapheme() const { return grapheme; }

  // @attention use this only when you guarantee that 'g' and 'grapheme' are from the same image
  // otherwise bad things will happen
  void  reuse_grapheme(const std::string_view& g) { grapheme = g; }

private:
  std::conditional_t<OWNERSHIP, std::string, std::string_view> grapheme = {};

public:
  // Colors:
  Color background_color = Color::Default;
  Color foreground_color = Color::Default;
};

using Pixel = TPixel<false>;
using PixelStandalone = TPixel<true>;

}  // namespace ftxui

#endif  // FTXUI_SCREEN_PIXEL_HPP
