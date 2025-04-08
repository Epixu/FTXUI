#pragma once
#include <cstdint>
#include <vector>
#include <string_view>

#if defined(_MSC_VER)
   #define FTXUI_FORCE_INLINE() __forceinline
#else
   #define FTXUI_FORCE_INLINE() __attribute__((always_inline)) inline
#endif

namespace ftxui {

/// @brief packs strings into an integer-size if possible
/// @attention used to represent pixel character data for images no bigger than 4096x4096
struct PackedString
{
  using Pool = std::vector<uint32_t>;

private:
  union {
    struct {
      uint32_t size    : 2;   // Size of the string, if small
      uint32_t big     : 1;   // Indicates whether pointer contains characters, or a reference to memory
      uint32_t pointer : 29;  // Either a 29bit pointer to the size and data of the string, or up to 3 characters
    };

    struct {
      uint8_t padding;        // Contains size and big
      uint8_t str[3];         // The three characters contained inside pointer
    };

    uint32_t all = 0;
  };

public:
  /// @brief Pack a character directly into the pointer
  FTXUI_FORCE_INLINE()
  constexpr PackedString(char c) {
    size = 1;
    pointer = c;
  }

  /// @brief Pack a small string directly into the pointer
  template<size_t S> FTXUI_FORCE_INLINE()
  constexpr PackedString(const char (&literal)[S]) {
    static_assert(S <= 4, "Can't contain a literal that is this big, use the alternative constructor");
    size = S - 1;
    for (size_t i = 0; i < S - 1; ++i)
      str[i] = literal[i];
  }
  
  /// @brief Pack a string by using the pool
  FTXUI_FORCE_INLINE()
  PackedString(const std::string_view& text, Pool& pool) {
    switch (text.size()) {
      case 0:
        return;

      case 1: case 2: case 3:
        size = text.size();
        for (size_t i = 0; i < text.size(); ++i)
          str[i] = text[i];
        break;

      default: {
        big = 1;
        pointer = pool.size();
        pool.resize(pool.size() + 1 + text.size() / 4 + text.size() % 4);
        pool[pointer] = static_cast<uint32_t>(text.size());
        auto as_char = reinterpret_cast<char*>(pool.data() + pointer + 1);
        for (size_t i = 0; i < text.size(); ++i)
          as_char[i] = text[i];
      }
    }    
  }

  FTXUI_FORCE_INLINE()
  bool empty() const {
     return size == 0;
  }

};

}
