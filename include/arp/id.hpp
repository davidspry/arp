#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace arp
{

template<size_t N> requires (N != 0)
struct Id final {
  std::array<char, N> buffer;

  constexpr Id(char x): buffer({x}) {}
  constexpr Id(const char(&str)[N]): buffer(std::to_array(str)) {}

  constexpr auto id() const -> std::string_view                  { return {buffer.data(), 1}; }
  constexpr auto id() const -> std::string_view requires (N > 1) { return {buffer.data(), N - 1}; }

  constexpr bool operator==(const Id&) const = default;

  template<size_t M>
  constexpr bool operator==(const Id<M>&) const { return false; }
};

Id(char) -> Id<1>;

}
