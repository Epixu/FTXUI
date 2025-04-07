#pragma once

namespace ftxui {

/// @brief packs strings into an integer-size if possible
/// @tparam ALIGNMENT - decides which least-significant bits to use for determining state
template<size_t ALIGNMENT_BITS>
struct PackedString
{
  static constexpr size_t AlignBitmask = ALIGNMENT;
};

/// Pixels' strings are always aligned to 4 bytes, which means that we can use 
/// last two bits for string state
using PackedString3 = PackedString<(1u << 2u) - 1>;

}