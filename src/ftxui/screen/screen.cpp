// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <cstddef>  // for size_t
#include <cstdint>
#include <iostream>  // for operator<<, stringstream, basic_ostream, flush, cout, ostream
#include <limits>
#include <map>      // for _Rb_tree_const_iterator, map, operator!=, operator==
#include <unordered_map>      // for _Rb_tree_const_iterator, map, operator!=, operator==
#include <sstream>  // IWYU pragma: keep
#include <utility>  // for pair

#include "ftxui/screen/image.hpp"  // for Image
#include "ftxui/screen/pixel.hpp"  // for Pixel
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"    // for string_width
#include "ftxui/screen/terminal.hpp"  // for Dimensions, Size

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// Macro for hinting that an expression is likely to be false.
#if !defined(FTXUI_UNLIKELY)
#if defined(COMPILER_GCC) || defined(__clang__)
#define FTXUI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define FTXUI_UNLIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(FTXUI_UNLIKELY)

#if !defined(FTXUI_LIKELY)
#if defined(COMPILER_GCC) || defined(__clang__)
#define FTXUI_LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define FTXUI_LIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(FTXUI_LIKELY)

namespace ftxui {

namespace {

#if defined(_WIN32)
void WindowsEmulateVT100Terminal() {
  static bool done = false;
  if (done)
    return;
  done = true;

  // Enable VT processing on stdout and stdin
  auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  DWORD out_mode = 0;
  GetConsoleMode(stdout_handle, &out_mode);

  // https://docs.microsoft.com/en-us/windows/console/setconsolemode
  const int enable_virtual_terminal_processing = 0x0004;
  const int disable_newline_auto_return = 0x0008;
  out_mode |= enable_virtual_terminal_processing;
  out_mode |= disable_newline_auto_return;

  SetConsoleMode(stdout_handle, out_mode);
}
#endif

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void UpdatePixelStyle(const Screen* screen,
                      std::stringstream& ss,
                      const Pixel* prev,
                      const Pixel* next) {
   if (!next) {
        // See https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda
        if (FTXUI_UNLIKELY(0 != prev->style.hyperlink)) {
          ss << "\x1B]8;;" << screen->Hyperlink(0) << "\x1B\\";
        }

        // Bold
        if (FTXUI_UNLIKELY(prev->style.bold | prev->style.dim)) {
          // BOLD_AND_DIM_RESET:
          ss << (prev->style.bold || prev->style.dim ? "\x1B[22m" : "");
        }

        // Underline
        if (FTXUI_UNLIKELY(0 != prev->style.underlined ||
                           0 != prev->style.underlined_double)) {
          ss << "\x1B[24m";  // UNDERLINE_RESET
        }

        // Blink
        if (FTXUI_UNLIKELY(0 != prev->style.blink)) {
          ss << "\x1B[25m";  // BLINK_RESET
        }

        // Inverted
        if (FTXUI_UNLIKELY(0 != prev->style.inverted)) {
          ss << "\x1B[27m";  // INVERTED_RESET
        }

        // Italics
        if (FTXUI_UNLIKELY(0 != prev->style.italic)) {
          ss << "\x1B[23m";  // ITALIC_RESET
        }

        // StrikeThrough
        if (FTXUI_UNLIKELY(0 != prev->style.strikethrough)) {
          ss << "\x1B[29m";  // CROSSED_OUT_RESET
        }

        if (FTXUI_UNLIKELY(Color(Color::Default) != prev->style.foreground_color ||
                           Color(Color::Default) != prev->style.background_color)) {
          ss << "\x1B[" + Color(Color::Default).Print(false) + "m";
          ss << "\x1B[" + Color(Color::Default).Print(true) + "m";
        }

      return;
   }

   if (!prev) {
     // See https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda
     if (FTXUI_UNLIKELY(next->style.hyperlink != 0)) {
       ss << "\x1B]8;;" << screen->Hyperlink(next->style.hyperlink) << "\x1B\\";
     }

     // Bold
     if (FTXUI_UNLIKELY(next->style.bold | next->style.dim)) {
       // BOLD_AND_DIM_RESET:
       ss << (next->style.bold ? "\x1B[1m" : "");  // BOLD_SET
       ss << (next->style.dim ? "\x1B[2m" : "");   // DIM_SET
     }

     // Underline
     if (FTXUI_UNLIKELY(next->style.underlined != 0 ||
                        next->style.underlined_double != 0)) {
       ss << (next->style.underlined          ? "\x1B[4m"     // UNDERLINE
              : next->style.underlined_double ? "\x1B[21m"    // UNDERLINE_DOUBLE
                                       : "\x1B[24m");  // UNDERLINE_RESET
     }

     // Blink
     if (FTXUI_UNLIKELY(next->style.blink != 0)) {
       ss << (next->style.blink ? "\x1B[5m"     // BLINK_SET
                         : "\x1B[25m");  // BLINK_RESET
     }

     // Inverted
     if (FTXUI_UNLIKELY(next->style.inverted != 0)) {
       ss << (next->style.inverted ? "\x1B[7m"     // INVERTED_SET
                            : "\x1B[27m");  // INVERTED_RESET
     }

     // Italics
     if (FTXUI_UNLIKELY(next->style.italic != 0)) {
       ss << (next->style.italic ? "\x1B[3m"     // ITALIC_SET
                          : "\x1B[23m");  // ITALIC_RESET
     }

     // StrikeThrough
     if (FTXUI_UNLIKELY(next->style.strikethrough != 0)) {
       ss << (next->style.strikethrough ? "\x1B[9m"     // CROSSED_OUT
                                 : "\x1B[29m");  // CROSSED_OUT_RESET
     }

     if (FTXUI_UNLIKELY(next->style.foreground_color != Color(Color::Default) ||
                        next->style.background_color != Color(Color::Default))) {
       ss << "\x1B[" + next->style.foreground_color.Print(false) + "m";
       ss << "\x1B[" + next->style.background_color.Print(true) + "m";
     }
     return;
   }

  // See https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda
  if (FTXUI_UNLIKELY(next->style.hyperlink != prev->style.hyperlink)) {
    ss << "\x1B]8;;" << screen->Hyperlink(next->style.hyperlink) << "\x1B\\";
  }

  // Bold
  if (FTXUI_UNLIKELY((next->style.bold ^ prev->style.bold) | (next->style.dim ^ prev->style.dim))) {
    // BOLD_AND_DIM_RESET:
    ss << ((prev->style.bold && !next->style.bold) || (prev->style.dim && !next->style.dim) ? "\x1B[22m"
                                                                : "");
    ss << (next->style.bold ? "\x1B[1m" : "");  // BOLD_SET
    ss << (next->style.dim ? "\x1B[2m" : "");   // DIM_SET
  }

  // Underline
  if (FTXUI_UNLIKELY(next->style.underlined != prev->style.underlined ||
                     next->style.underlined_double != prev->style.underlined_double)) {
    ss << (next->style.underlined          ? "\x1B[4m"     // UNDERLINE
           : next->style.underlined_double ? "\x1B[21m"    // UNDERLINE_DOUBLE
                                    : "\x1B[24m");  // UNDERLINE_RESET
  }

  // Blink
  if (FTXUI_UNLIKELY(next->style.blink != prev->style.blink)) {
    ss << (next->style.blink ? "\x1B[5m"     // BLINK_SET
                      : "\x1B[25m");  // BLINK_RESET
  }

  // Inverted
  if (FTXUI_UNLIKELY(next->style.inverted != prev->style.inverted)) {
    ss << (next->style.inverted ? "\x1B[7m"     // INVERTED_SET
                         : "\x1B[27m");  // INVERTED_RESET
  }

  // Italics
  if (FTXUI_UNLIKELY(next->style.italic != prev->style.italic)) {
    ss << (next->style.italic ? "\x1B[3m"     // ITALIC_SET
                       : "\x1B[23m");  // ITALIC_RESET
  }

  // StrikeThrough
  if (FTXUI_UNLIKELY(next->style.strikethrough != prev->style.strikethrough)) {
    ss << (next->style.strikethrough ? "\x1B[9m"     // CROSSED_OUT
                              : "\x1B[29m");  // CROSSED_OUT_RESET
  }

  if (FTXUI_UNLIKELY(next->style.foreground_color != prev->style.foreground_color ||
                     next->style.background_color != prev->style.background_color)) {
    ss << "\x1B[" + next->style.foreground_color.Print(false) + "m";
    ss << "\x1B[" + next->style.background_color.Print(true) + "m";
  }
}

struct TileEncoding {
  std::uint8_t left : 2;
  std::uint8_t top : 2;
  std::uint8_t right : 2;
  std::uint8_t down : 2;
  std::uint8_t round : 1;

  // clang-format off
  bool operator<(const TileEncoding& other) const {
    if (left < other.left)   { return  true;  }
    if (left > other.left)   { return  false; }
    if (top < other.top)     { return  true;  }
    if (top > other.top)     { return  false; }
    if (right < other.right) { return  true;  }
    if (right > other.right) { return  false; }
    if (down < other.down)   { return  true;  }
    if (down > other.down)   { return  false; }
    if (round < other.round) { return  true;  }
    if (round > other.round) { return  false; }
      return false;
  }
  // clang-format on
};

// clang-format off
const std::unordered_map<PackedString, TileEncoding> tile_encoding = { // NOLINT
    {"─", {1, 0, 1, 0, 0}},
    {"━", {2, 0, 2, 0, 0}},
    {"╍", {2, 0, 2, 0, 0}},

    {"│", {0, 1, 0, 1, 0}},
    {"┃", {0, 2, 0, 2, 0}},
    {"╏", {0, 2, 0, 2, 0}},

    {"┌", {0, 0, 1, 1, 0}},
    {"┍", {0, 0, 2, 1, 0}},
    {"┎", {0, 0, 1, 2, 0}},
    {"┏", {0, 0, 2, 2, 0}},

    {"┐", {1, 0, 0, 1, 0}},
    {"┑", {2, 0, 0, 1, 0}},
    {"┒", {1, 0, 0, 2, 0}},
    {"┓", {2, 0, 0, 2, 0}},

    {"└", {0, 1, 1, 0, 0}},
    {"┕", {0, 1, 2, 0, 0}},
    {"┖", {0, 2, 1, 0, 0}},
    {"┗", {0, 2, 2, 0, 0}},

    {"┘", {1, 1, 0, 0, 0}},
    {"┙", {2, 1, 0, 0, 0}},
    {"┚", {1, 2, 0, 0, 0}},
    {"┛", {2, 2, 0, 0, 0}},

    {"├", {0, 1, 1, 1, 0}},
    {"┝", {0, 1, 2, 1, 0}},
    {"┞", {0, 2, 1, 1, 0}},
    {"┟", {0, 1, 1, 2, 0}},
    {"┠", {0, 2, 1, 2, 0}},
    {"┡", {0, 2, 2, 1, 0}},
    {"┢", {0, 1, 2, 2, 0}},
    {"┣", {0, 2, 2, 2, 0}},

    {"┤", {1, 1, 0, 1, 0}},
    {"┥", {2, 1, 0, 1, 0}},
    {"┦", {1, 2, 0, 1, 0}},
    {"┧", {1, 1, 0, 2, 0}},
    {"┨", {1, 2, 0, 2, 0}},
    {"┩", {2, 2, 0, 1, 0}},
    {"┪", {2, 1, 0, 2, 0}},
    {"┫", {2, 2, 0, 2, 0}},

    {"┬", {1, 0, 1, 1, 0}},
    {"┭", {2, 0, 1, 1, 0}},
    {"┮", {1, 0, 2, 1, 0}},
    {"┯", {2, 0, 2, 1, 0}},
    {"┰", {1, 0, 1, 2, 0}},
    {"┱", {2, 0, 1, 2, 0}},
    {"┲", {1, 0, 2, 2, 0}},
    {"┳", {2, 0, 2, 2, 0}},

    {"┴", {1, 1, 1, 0, 0}},
    {"┵", {2, 1, 1, 0, 0}},
    {"┶", {1, 1, 2, 0, 0}},
    {"┷", {2, 1, 2, 0, 0}},
    {"┸", {1, 2, 1, 0, 0}},
    {"┹", {2, 2, 1, 0, 0}},
    {"┺", {1, 2, 2, 0, 0}},
    {"┻", {2, 2, 2, 0, 0}},

    {"┼", {1, 1, 1, 1, 0}},
    {"┽", {2, 1, 1, 1, 0}},
    {"┾", {1, 1, 2, 1, 0}},
    {"┿", {2, 1, 2, 1, 0}},
    {"╀", {1, 2, 1, 1, 0}},
    {"╁", {1, 1, 1, 2, 0}},
    {"╂", {1, 2, 1, 2, 0}},
    {"╃", {2, 2, 1, 1, 0}},
    {"╄", {1, 2, 2, 1, 0}},
    {"╅", {2, 1, 1, 2, 0}},
    {"╆", {1, 1, 2, 2, 0}},
    {"╇", {2, 2, 2, 1, 0}},
    {"╈", {2, 1, 2, 2, 0}},
    {"╉", {2, 2, 1, 2, 0}},
    {"╊", {1, 2, 2, 2, 0}},
    {"╋", {2, 2, 2, 2, 0}},

    {"═", {3, 0, 3, 0, 0}},
    {"║", {0, 3, 0, 3, 0}},

    {"╒", {0, 0, 3, 1, 0}},
    {"╓", {0, 0, 1, 3, 0}},
    {"╔", {0, 0, 3, 3, 0}},

    {"╕", {3, 0, 0, 1, 0}},
    {"╖", {1, 0, 0, 3, 0}},
    {"╗", {3, 0, 0, 3, 0}},

    {"╘", {0, 1, 3, 0, 0}},
    {"╙", {0, 3, 1, 0, 0}},
    {"╚", {0, 3, 3, 0, 0}},

    {"╛", {3, 1, 0, 0, 0}},
    {"╜", {1, 3, 0, 0, 0}},
    {"╝", {3, 3, 0, 0, 0}},

    {"╞", {0, 1, 3, 1, 0}},
    {"╟", {0, 3, 1, 3, 0}},
    {"╠", {0, 3, 3, 3, 0}},

    {"╡", {3, 1, 0, 1, 0}},
    {"╢", {1, 3, 0, 3, 0}},
    {"╣", {3, 3, 0, 3, 0}},

    {"╤", {3, 0, 3, 1, 0}},
    {"╥", {1, 0, 1, 3, 0}},
    {"╦", {3, 0, 3, 3, 0}},

    {"╧", {3, 1, 3, 0, 0}},
    {"╨", {1, 3, 1, 0, 0}},
    {"╩", {3, 3, 3, 0, 0}},

    {"╪", {3, 1, 3, 1, 0}},
    {"╫", {1, 3, 1, 3, 0}},
    {"╬", {3, 3, 3, 3, 0}},

    {"╭", {0, 0, 1, 1, 1}},
    {"╮", {1, 0, 0, 1, 1}},
    {"╯", {1, 1, 0, 0, 1}},
    {"╰", {0, 1, 1, 0, 1}},

    {"╴", {1, 0, 0, 0, 0}},
    {"╵", {0, 1, 0, 0, 0}},
    {"╶", {0, 0, 1, 0, 0}},
    {"╷", {0, 0, 0, 1, 0}},

    {"╸", {2, 0, 0, 0, 0}},
    {"╹", {0, 2, 0, 0, 0}},
    {"╺", {0, 0, 2, 0, 0}},
    {"╻", {0, 0, 0, 2, 0}},

    {"╼", {1, 0, 2, 0, 0}},
    {"╽", {0, 1, 0, 2, 0}},
    {"╾", {2, 0, 1, 0, 0}},
    {"╿", {0, 2, 0, 1, 0}},
};
// clang-format on

template <class A, class B>
std::map<B, A> InvertMap(const std::unordered_map<A, B> input) {
  std::map<B, A> output;
  for (const auto& it : input) {
    output[it.second] = it.first;
  }
  return output;
}

const std::map<TileEncoding, PackedString> tile_encoding_inverse =  // NOLINT
    InvertMap(tile_encoding);

}  // namespace

/// A fixed dimension.
/// @see Fit
/// @see Full
Dimensions Dimension::Fixed(int v) {
  return {v, v};
}

/// Use the terminal dimensions.
/// @see Fixed
/// @see Fit
Dimensions Dimension::Full() {
  return Terminal::Size();
}

// static
/// Create a screen with the given dimension along the x-axis and y-axis.
Screen Screen::Create(Dimensions width, Dimensions height) {
  return {width.dimx, height.dimy};
}

// static
/// Create a screen with the given dimension.
Screen Screen::Create(Dimensions dimension) {
  return {dimension.dimx, dimension.dimy};
}

Screen::Screen(int dimx, int dimy) : Image{dimx, dimy} {
#if defined(_WIN32)
  // The placement of this call is a bit weird, however we can assume that
  // anybody who instantiates a Screen object eventually wants to output
  // something to the console. If that is not the case, use an instance of Image
  // instead. As we require UTF8 for all input/output operations we will just
  // switch to UTF8 encoding here
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  WindowsEmulateVT100Terminal();
#endif
}

// @attention assumes both pixels are small
void Screen::UpgradeLeftRight(Pixel& left, Pixel& right) {
  const auto it_left = tile_encoding.find(left.grapheme);
  if (it_left == tile_encoding.end())
    return;

  const auto it_right = tile_encoding.find(right.grapheme);
  if (it_right == tile_encoding.end())
    return;

  if (it_left->second.right == 0 && it_right->second.left != 0) {
    TileEncoding encoding_left = it_left->second;
    encoding_left.right = it_right->second.left;
    const auto it_left_upgrade = tile_encoding_inverse.find(encoding_left);
    if (it_left_upgrade != tile_encoding_inverse.end()) {
      left.grapheme = it_left_upgrade->second;
    }
  }

  if (it_right->second.left == 0 && it_left->second.right != 0) {
    TileEncoding encoding_right = it_right->second;
    encoding_right.left = it_left->second.right;
    const auto it_right_upgrade = tile_encoding_inverse.find(encoding_right);
    if (it_right_upgrade != tile_encoding_inverse.end()) {
      right.grapheme = it_right_upgrade->second;
    }
  }
}

// @attention assumes both pixels are small
void Screen::UpgradeTopDown(Pixel& top, Pixel& down) {
  const auto it_top = tile_encoding.find(top.grapheme);
  if (it_top == tile_encoding.end())
    return;

  const auto it_down = tile_encoding.find(down.grapheme);
  if (it_down == tile_encoding.end())
    return;

  if (it_top->second.down == 0 && it_down->second.top != 0) {
    TileEncoding encoding_top = it_top->second;
    encoding_top.down = it_down->second.top;
    const auto it_top_down = tile_encoding_inverse.find(encoding_top);
    if (it_top_down != tile_encoding_inverse.end()) {
      top.grapheme = it_top_down->second;
    }
  }

  if (it_down->second.top == 0 && it_top->second.down != 0) {
    TileEncoding encoding_down = it_down->second;
    encoding_down.top = it_top->second.down;
    const auto it_down_top = tile_encoding_inverse.find(encoding_down);
    if (it_down_top != tile_encoding_inverse.end()) {
      down.grapheme = it_down_top->second;
    }
  }
}

/// Produce a std::string that can be used to print the Screen on the
/// terminal.
/// @note Don't forget to flush stdout. Alternatively, you can use
/// Screen::Print();
std::string Screen::ToString() const {
  std::stringstream ss;
  const Pixel* previous_pixel_ref = nullptr;

  for (int y = 0; y < dimy_; ++y) {
    // New line in between two lines.
    if (y != 0) {
      UpdatePixelStyle(this, ss, previous_pixel_ref, nullptr);
      previous_pixel_ref = nullptr;
      ss << "\r\n";
    }

    // After printing a fullwith character, we need to skip the next cell.
    bool previous_fullwidth = false;
    auto pixel_row = pixels_.data() + y * width();
    for (auto pixel = pixel_row; pixel < pixel_row + width(); ++pixel) {
      if (!previous_fullwidth) {
        UpdatePixelStyle(this, ss, previous_pixel_ref, pixel);
        previous_pixel_ref = pixel;
        if (pixel->grapheme.empty()) {
          ss << ' ';
        } else {
          ss << pixel->grapheme.get_view(pool_);
        }
      }
      previous_fullwidth = (string_width(pixel->grapheme.get_view(pool_)) == 2);
    }
  }

  // Reset the style to default:
  UpdatePixelStyle(this, ss, previous_pixel_ref, nullptr);

  return ss.str();
}

// Print the Screen to the terminal.
void Screen::Print() const {
  std::cout << ToString() << '\0' << std::flush;
}

/// @brief Return a string to be printed in order to reset the cursor position
///        to the beginning of the screen.
///
/// ```cpp
/// std::string reset_position;
/// while(true) {
///   auto document = render();
///   auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
///   Render(screen, document);
///   std::cout << reset_position << screen.ToString() << std::flush;
///   reset_position = screen.ResetPosition();
///
///   using namespace std::chrono_literals;
///   std::this_thread::sleep_for(0.01s);
/// }
/// ```
///
/// @return The string to print in order to reset the cursor position to the
///         beginning.
std::string Screen::ResetPosition(bool clear) const {
  std::stringstream ss;
  if (clear) {
    ss << "\r";       // MOVE_LEFT;
    ss << "\x1b[2K";  // CLEAR_SCREEN;
    for (int y = 1; y < dimy_; ++y) {
      ss << "\x1B[1A";  // MOVE_UP;
      ss << "\x1B[2K";  // CLEAR_LINE;
    }
  } else {
    ss << "\r";  // MOVE_LEFT;
    for (int y = 1; y < dimy_; ++y) {
      ss << "\x1B[1A";  // MOVE_UP;
    }
  }
  return ss.str();
}

/// @brief Clear all the pixel from the screen.
void Screen::Clear() {
  Image::Clear();

  cursor_.x = dimx_ - 1;
  cursor_.y = dimy_ - 1;

  hyperlinks_ = {
      "",
  };
}

// clang-format off
void Screen::ApplyShader() {
  // Merge box characters together
  Pixel* data = pixels_.data() + 1;

  for (int x = 1; x < dimx_; ++x) {
     // Box drawing character uses exactly 3 byte.
     if (!data->ShouldAttemptAutoMerge()) {
        ++data;
        continue;
     }

     Pixel* left = data - 1;
     if (left->ShouldAttemptAutoMerge())
        UpgradeLeftRight(*left, *data);

     ++data;
  }

  for (int y = 1; y < dimy_; ++y) {
    for (int x = 0; x < dimx_; ++x) {
      // Box drawing character uses exactly 3 byte.
      if (!data->ShouldAttemptAutoMerge()) {
        ++data;
        continue;
      }

      if (x > 0) {
        Pixel* left = data - 1;
        if (left->ShouldAttemptAutoMerge())
          UpgradeLeftRight(*left, *data);
      }

      Pixel* top = data - dimx_;
      if (top->ShouldAttemptAutoMerge())
         UpgradeTopDown(*top, *data);

      ++data;
    }
  }
}
// clang-format on

std::uint8_t Screen::RegisterHyperlink(const std::string& link) {
  for (std::size_t i = 0; i < hyperlinks_.size(); ++i) {
    if (hyperlinks_[i] == link) {
      return i;
    }
  }
  if (hyperlinks_.size() == std::numeric_limits<std::uint8_t>::max()) {
    return 0;
  }
  hyperlinks_.push_back(link);
  return hyperlinks_.size() - 1;
}

const std::string& Screen::Hyperlink(std::uint8_t id) const {
  if (id >= hyperlinks_.size()) {
    return hyperlinks_[0];
  }
  return hyperlinks_[id];
}

/// @brief Return the current selection style.
/// @see SetSelectionStyle
const Screen::SelectionStyle& Screen::GetSelectionStyle() const {
  return selection_style_;
}

/// @brief Set the current selection style.
/// @see GetSelectionStyle
void Screen::SetSelectionStyle(SelectionStyle decorator) {
  selection_style_ = std::move(decorator);
}

}  // namespace ftxui
