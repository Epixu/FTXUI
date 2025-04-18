// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef FTXUI_DOM_CANVAS_HPP
#define FTXUI_DOM_CANVAS_HPP

#include <cstddef>        // for size_t
#include <functional>     // for function
#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "ftxui/screen/color.hpp"  // for Color
#include "ftxui/screen/image.hpp"  // for Pixel, Image

#ifdef DrawText
// Workaround for WinUsr.h (via Windows.h) defining macros that break things.
// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-drawtext
#undef DrawText
#endif

namespace ftxui {

struct Canvas : Image {
 public:
  Canvas() = delete;
  Canvas(int width, int height);
  Canvas(const Canvas&) = default;
  Canvas(Canvas&&) = default;

  Canvas& operator = (const Canvas&) = default;
  Canvas& operator = (Canvas&&) = default;

  // Getters: now coming from base
  int subpixel_width() const { return Image::width() * 2; }
  int subpixel_height() const { return Image::height() * 4; }
  //Pixel GetPixel(int x, int y) const;

  using Stylizer = std::function<void(Pixel&)>;

  // Draws using braille characters --------------------------------------------
  void DrawPointOn(int x, int y);
  void DrawPointOff(int x, int y);
  void DrawPointToggle(int x, int y);
  void DrawPoint(int x, int y, bool value);
  void DrawPoint(int x, int y, bool value, const Stylizer& s);
  void DrawPoint(int x, int y, bool value, const Color& color);
  void DrawPointLine(int x1, int y1, int x2, int y2);
  void DrawPointLine(int x1, int y1, int x2, int y2, const Stylizer& s);
  void DrawPointLine(int x1, int y1, int x2, int y2, const Color& color);
  void DrawPointCircle(int x, int y, int radius);
  void DrawPointCircle(int x, int y, int radius, const Stylizer& s);
  void DrawPointCircle(int x, int y, int radius, const Color& color);
  void DrawPointCircleFilled(int x, int y, int radius);
  void DrawPointCircleFilled(int x, int y, int radius, const Stylizer& s);
  void DrawPointCircleFilled(int x, int y, int radius, const Color& color);
  void DrawPointEllipse(int x, int y, int r1, int r2);
  void DrawPointEllipse(int x, int y, int r1, int r2, const Color& color);
  void DrawPointEllipse(int x, int y, int r1, int r2, const Stylizer& s);
  void DrawPointEllipseFilled(int x, int y, int r1, int r2);
  void DrawPointEllipseFilled(int x, int y, int r1, int r2, const Color& color);
  void DrawPointEllipseFilled(int x, int y, int r1, int r2, const Stylizer& s);

  // Draw using box characters -------------------------------------------------
  // Block are of size 1x2. y is considered to be a multiple of 2.
  void DrawBlockOn(int x, int y);
  void DrawBlockOff(int x, int y);
  void DrawBlockToggle(int x, int y);
  void DrawBlock(int x, int y, bool value);
  void DrawBlock(int x, int y, bool value, const Stylizer& s);
  void DrawBlock(int x, int y, bool value, const Color& color);
  void DrawBlockLine(int x1, int y1, int x2, int y2);
  void DrawBlockLine(int x1, int y1, int x2, int y2, const Stylizer& s);
  void DrawBlockLine(int x1, int y1, int x2, int y2, const Color& color);
  void DrawBlockCircle(int x1, int y1, int radius);
  void DrawBlockCircle(int x1, int y1, int radius, const Stylizer& s);
  void DrawBlockCircle(int x1, int y1, int radius, const Color& color);
  void DrawBlockCircleFilled(int x1, int y1, int radius);
  void DrawBlockCircleFilled(int x1, int y1, int radius, const Stylizer& s);
  void DrawBlockCircleFilled(int x1, int y1, int radius, const Color& color);
  void DrawBlockEllipse(int x1, int y1, int r1, int r2);
  void DrawBlockEllipse(int x1, int y1, int r1, int r2, const Stylizer& s);
  void DrawBlockEllipse(int x1, int y1, int r1, int r2, const Color& color);
  void DrawBlockEllipseFilled(int x1, int y1, int r1, int r2);
  void DrawBlockEllipseFilled(int x1, int y1,
                              int r1, int r2,
                              const Stylizer& s);
  void DrawBlockEllipseFilled(int x1, int y1,
                              int r1, int r2,
                              const Color& color);

  // Draw using normal characters ----------------------------------------------
  // Draw using character of size 2x4 at position (x,y)
  // x is considered to be a multiple of 2.
  // y is considered to be a multiple of 4.
  void DrawText(int x, int y, const std::string_view& value);
  void DrawText(int x, int y, const std::string_view& value, const Color& color);
  void DrawText(int x, int y, const std::string_view& value, const Stylizer& style);

  // Draw using directly pixels or images --------------------------------------
  // x is considered to be a multiple of 2.
  // y is considered to be a multiple of 4.
  void DrawPixel(int x, int y, const PixelStandalone&);
  void DrawImage(int x, int y, const Image&);

  // Decorator:
  // x is considered to be a multiple of 2.
  // y is considered to be a multiple of 4.
  void Style(int x, int y, const Stylizer& style);

 private:
  bool IsIn(int x, int y) const {
    return x >= 0 && x < dimx_ && y >= 0 && y < dimy_;
  }

  struct Cell {
    enum {
      kCell,     // Units of size 2x4
      kBlock,    // Units of size 2x2
      kBraille,  // Units of size 1x1
    } type = kCell;
  };

  // Additional meta data - type of cell for each pixel
  std::vector<Cell> cells_;

  // No point in using a map for indexing - lots of table overhead and cache unfriendliness
  // just index linearly: i = x + y*width. nothing will ever be faster and friendlier than that, ever
  //std::unordered_map<XY, Cell, XYHash> storage_;
};

}  // namespace ftxui

#endif  // FTXUI_DOM_CANVAS_HPP
