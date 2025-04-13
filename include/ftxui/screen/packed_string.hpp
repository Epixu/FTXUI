#pragma once
#include <cstdint>
#include <cstring>
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
      uint32_t pointer : 29;  // Either a 29bit pointer to the size and data of the string, or up to 3 characters
      uint32_t size    : 2;   // Size of the string, if small - up to 3 characters
      uint32_t big     : 1;   // Indicates whether pointer contains characters, or a reference to memory
    };

    struct {
      char str[3];            // The three characters contained inside pointer (small value optimization)
      char padding;           // Contains size and big
    };

    uint32_t all = 0;
  };

public:
  /// @brief Pack a character directly into the pointer
  FTXUI_FORCE_INLINE()
  constexpr PackedString() : all(0) { }

  /// @brief Pack a character directly into the pointer
  FTXUI_FORCE_INLINE()
  constexpr PackedString(char c) {
    size = 1;
    str[0] = c;
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
  void copy(const std::string_view& text, Pool& pool) {
    switch (text.size()) {
      case 0:
        all = 0;
        break;

      case 1: case 2: case 3:
        big = 0;
        size = text.size();
        for (size_t i = 0; i < text.size(); ++i)
          str[i] = text[i];
        break;

      default: {
        const auto aligned_size = text.size() + text.size() % 4u;
        if (big && pool[pointer] <= aligned_size) {
           // We can reuse the pool allocation
           pool[pointer] = static_cast<uint32_t>(text.size());
           auto as_char = reinterpret_cast<char*>(pool.data() + pointer + 1);
           for (size_t i = 0; i < text.size(); ++i)
              as_char[i] = text[i];
        }
        else {
           big = 1;
           pointer = pool.size();
           pool.resize(pool.size() + 1 + aligned_size / 4);
           pool[pointer] = static_cast<uint32_t>(text.size());
           auto as_char = reinterpret_cast<char*>(pool.data() + pointer + 1);
           for (size_t i = 0; i < text.size(); ++i)
              as_char[i] = text[i];
        }
      }
    }    
  }

  FTXUI_FORCE_INLINE()
  bool empty() const {
     return size == 0 && !big;
  }

  FTXUI_FORCE_INLINE()
  auto get_size() const {
     return size;
  }

  FTXUI_FORCE_INLINE()
  bool is_small() const {
     return !big;
  }

  FTXUI_FORCE_INLINE()
  std::string_view get_view(const Pool& pool) const {
     if (big) {
        return std::string_view(
           reinterpret_cast<const char*>(pool.data() + pointer + 1),
           static_cast<size_t>(pool[pointer])
        );
     }
     else {
        (void)pool;
        return std::string_view(str, size);
     }
  }

  // @attention this is unsafe, use only if you can guarantee !big
  FTXUI_FORCE_INLINE()
  std::string_view get_view() const {
      return std::string_view(str, size);
  }

  // @attention this is unsafe, use only if you can guarantee !big
  FTXUI_FORCE_INLINE()
  bool operator == (const PackedString& rhs) const {
      return size == rhs.size && strncmp(str, rhs.str, size);
  }

  // @attention this is unsafe, use only if you can guarantee !big
  FTXUI_FORCE_INLINE()
  char& operator [] (size_t i) {
     return str[i];
  }

  // @attention this is unsafe, use only if you can guarantee rhs.size() <= 3
  FTXUI_FORCE_INLINE()
  PackedString& operator = (const std::string_view& rhs) {
     big = 0;
     size = rhs.size();
     for (size_t i = 0; i < rhs.size(); ++i)
        str[i] = rhs[i];
     return *this;
  }

  /// @brief Pack a small string directly into the pointer
  template<size_t S> FTXUI_FORCE_INLINE()
  PackedString& operator = (const char (&literal)[S]) {
    static_assert(S <= 4, "Can't contain a literal that is this big, use the alternative operator");
    size = S - 1;
    for (size_t i = 0; i < S - 1; ++i)
      str[i] = literal[i];
    return *this;
  }

};

}

namespace std
{
   template<>
   struct hash<ftxui::PackedString> {
      FTXUI_FORCE_INLINE()
      size_t operator()(const ftxui::PackedString& what) const noexcept {
         return hash<std::string_view>()(what.get_view());
      }
   };
}