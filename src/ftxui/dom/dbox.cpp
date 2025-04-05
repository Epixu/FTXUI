// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <algorithm>  // for max
#include <cstddef>    // for size_t
#include <memory>     // for __shared_ptr_access, shared_ptr, make_shared
#include <utility>    // for move
#include <vector>

#include "ftxui/dom/elements.hpp"     // for Element, Elements, dbox
#include "ftxui/dom/node.hpp"         // for Node, Elements
#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box
#include "ftxui/screen/pixel.hpp"     // for Pixel

namespace ftxui {

namespace {
class DBox : public Node {
 public:
  explicit DBox(Elements children) : Node(std::move(children)) {}

  void ComputeRequirement() override {
    requirement_ = Requirement{};
    for (auto& child : children_) {
      child->ComputeRequirement();

      // Propagate the focused requirement.
      if (requirement_.focused.Prefer(child->requirement().focused)) {
        requirement_.focused = child->requirement().focused;
      }

      // Extend the min_x and min_y to contain all the children
      requirement_.min_x =
          std::max(requirement_.min_x, child->requirement().min_x);
      requirement_.min_y =
          std::max(requirement_.min_y, child->requirement().min_y);
    }
  }

  void SetBox(Box box) override {
    Node::SetBox(box);

    for (auto& child : children_) {
      child->SetBox(box);
    }
  }

  void Render(Screen& screen) override {
    if (children_.size() <= 1) {
      Node::Render(screen);
      return;
    }

    const int y_min = std::max(box_.y_min, 0);
    const int y_max = std::min(screen.height(), box_.y_max + 1);
    const int x_min = std::max(box_.x_min, 0);
    const int x_max = std::min(screen.width(), box_.x_max + 1);
    const int width = x_max - x_min;// +1;
    const int height = y_max - y_min;// +1;

    // Previously, this temporary pixel array also included regions outside screen,
    // which is a completely unnecessary overhead. You rely on screen.PixelAt to avoid
    // painting these pixels, but they were still computed for no reason. additioanlly screen.PixelAt no longer needs to be safe, if proper precausions are taken beforehand
    Image pixels(width, height);

    for (auto& child : children_) {
      child->Render(screen);

      // Accumulate the pixels of each child
      Pixel* acc = pixels.get_pixels().data();
      for (int y = y_min; y < y_max; ++y) {
        for (int x = x_min; x < x_max; ++x) { // iterate x as inner for cache-friendliness
          auto& pixel = screen.PixelAt(x, y);
          acc->background_color = Color::Blend(acc->background_color, pixel.background_color);
          acc->automerge = pixel.automerge || acc->automerge;

          if (pixel.get_grapheme().empty()) {
            acc->foreground_color = Color::Blend(acc->foreground_color, pixel.background_color); //TODO graphemes are never empty() currently. this can be fixed by a custom string_view class that allows for size() == 0 without resetting the data() pointer
          } else {
            acc->blink = pixel.blink;
            acc->bold = pixel.bold;
            acc->dim = pixel.dim;
            acc->inverted = pixel.inverted;
            acc->italic = pixel.italic;
            acc->underlined = pixel.underlined;
            acc->underlined_double = pixel.underlined_double;
            acc->strikethrough = pixel.strikethrough;
            acc->hyperlink = pixel.hyperlink;
            acc->set_grapheme(pixel.get_grapheme(), pixels);
            acc->foreground_color = pixel.foreground_color;
          }

          ++acc;  // NOLINT

          pixel.reset(); //TODO no longer makes grapheme empty, but instead does ' '
        }
      }
    }

    // Render the accumulated pixels back onto the screen:
    Pixel* acc = pixels.get_pixels().data();
    for (int y = y_min; y < y_max; ++y) {
       for (int x = x_min; x < x_max; ++x) {
          screen.PixelAt(x_min, y).copy_pixel_data(*acc, screen); // NOLINT
          ++acc;
       }
    }
  }
};
}  // namespace

/// @brief Stack several element on top of each other.
/// @param children_ The input element.
/// @return The right aligned element.
/// @ingroup dom
Element dbox(Elements children_) {
  return std::make_shared<DBox>(std::move(children_));
}

}  // namespace ftxui
