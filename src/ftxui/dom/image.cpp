// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include "ftxui/screen/image.hpp"
#include "ftxui/dom/node.hpp"         // for Node
#include "ftxui/util/ref.hpp"         // for ConstRef

namespace ftxui {

namespace {

class ImageNodeBase : public Node {
 public:
  ImageNodeBase() = default;

  void Render(Screen& screen) override {
    const Image& c = image();
    const int y_max = std::min(c.height(), box_.y_max - box_.y_min + 1);
    const int x_max = std::min(c.width(), box_.x_max - box_.x_min + 1);
    for (int y = 0; y < y_max; ++y) {
      for (int x = 0; x < x_max; ++x) {
        screen.PixelAt(box_.x_min + x, box_.y_min + y).copy(c.PixelAt(x, y), screen.get_pool(), c.get_pool());
      }
    }
  }

  virtual const Image& image() = 0;
};

}  // namespace

/// @brief Produce an element from an Image, or a reference to an Image.
// NOLINTNEXTLINE
Element image(ConstRef<Image> image) {
  class Impl : public ImageNodeBase {
   public:
    explicit Impl(ConstRef<Image> image) : image_(std::move(image)) {
      requirement_.min_x = (image_->width() + 1) / 2;
      requirement_.min_y = (image_->height() + 3) / 4;
    }
    const Image& image() final { return *image_; }
    ConstRef<Image> image_;
  };
  return std::make_shared<Impl>(image);
}

/// @brief Produce an element drawing an image of requested size.
/// @param width the width of the image.
/// @param height the height of the image.
/// @param fn a function drawing the image.
Element image(int width, int height, std::function<void(Image&)> fn) {
  class Impl : public ImageNodeBase {
   public:
    Impl(int width, int height, std::function<void(Image&)> fn)
        : image_(width, height), fn_(std::move(fn)) {}

    void ComputeRequirement() final {
      requirement_.min_x = (image_.width() + 1) / 2;
      requirement_.min_y = (image_.height() + 3) / 4;
    }

    void Render(Screen& screen) final {
      const int width = (box_.x_max - box_.x_min + 1) * 2;
      const int height = (box_.y_max - box_.y_min + 1) * 4;
      image_ = Image(width, height);
      fn_(image_);
      ImageNodeBase::Render(screen);
    }

    const Image& image() final { return image_; }
    Image image_;
    std::function<void(Image&)> fn_;
  };
  return std::make_shared<Impl>(width, height, std::move(fn));
}

/// @brief Produce an element drawing a canvas.
/// @param fn a function drawing the canvas.
Element image(std::function<void(Image&)> fn) {
  const int default_dim = 12;
  return image(default_dim, default_dim, std::move(fn));
}

}  // namespace ftxui
