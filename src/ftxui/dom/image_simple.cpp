// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include "ftxui/screen/image_simple.hpp"
#include "ftxui/dom/node.hpp"         // for Node
#include "ftxui/util/ref.hpp"         // for ConstRef

namespace ftxui {

namespace {

class ImageSimpleNodeBase : public Node {
 public:
    ImageSimpleNodeBase() = default;

  void Render(Screen& screen) override {
    const ImageSimple& c = image_simple();
    const int y_max = std::min(c.height(), box_.y_max - box_.y_min + 1);
    const int x_max = std::min(c.width(), box_.x_max - box_.x_min + 1);
    for (int y = 0; y < y_max; ++y) {
      for (int x = 0; x < x_max; ++x) {
         auto& p = screen.PixelAt(box_.x_min + x, box_.y_min + y); // .copy(c.PixelAt(x, y), screen.get_pool(), c.get_pool());
         p.grapheme = c.CharAt(x, y);
         p.style.background_color = c.ColorAt(x, y);
      }
    }
  }

  virtual const ImageSimple& image_simple() = 0;
};

}  // namespace

/// @brief Produce an element from an Image, or a reference to an Image.
// NOLINTNEXTLINE
Element image_simple(ConstRef<ImageSimple> image) {
  class Impl : public ImageSimpleNodeBase {
   public:
    explicit Impl(ConstRef<ImageSimple> image) : image_simple_(std::move(image)) {
      requirement_.min_x = (image_simple_->width() + 1) / 2;
      requirement_.min_y = (image_simple_->height() + 3) / 4;
    }
    const ImageSimple& image_simple() final { return *image_simple_; }
    ConstRef<ImageSimple> image_simple_;
  };
  return std::make_shared<Impl>(image);
}

/// @brief Produce an element drawing an image of requested size.
/// @param width the width of the image.
/// @param height the height of the image.
/// @param fn a function drawing the image.
Element image_simple(int width, int height, std::function<void(ImageSimple&)> fn) {
  class Impl : public ImageSimpleNodeBase {
   public:
    Impl(int width, int height, std::function<void(ImageSimple&)> fn)
        : image_simple_(width, height), fn_(std::move(fn)) {}

    void ComputeRequirement() final {
      requirement_.min_x = (image_simple_.width() + 1) / 2;
      requirement_.min_y = (image_simple_.height() + 3) / 4;
    }

    void Render(Screen& screen) final {
      const int width = (box_.x_max - box_.x_min + 1) * 2;
      const int height = (box_.y_max - box_.y_min + 1) * 4;
      image_simple_ = ImageSimple(width, height);
      fn_(image_simple_);
      ImageSimpleNodeBase::Render(screen);
    }

    const ImageSimple& image_simple() final { return image_simple_; }
    ImageSimple image_simple_;
    std::function<void(ImageSimple&)> fn_;
  };
  return std::make_shared<Impl>(width, height, std::move(fn));
}

/// @brief Produce an element drawing a canvas.
/// @param fn a function drawing the canvas.
Element image_simple(std::function<void(ImageSimple&)> fn) {
  const int default_dim = 12;
  return image_simple(default_dim, default_dim, std::move(fn));
}

}  // namespace ftxui
