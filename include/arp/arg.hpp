#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <array>
#include <cstddef>
#include <string_view>
#include <tuple>
#include <utility>

namespace arp
{

template<size_t N, Id... K> requires (sizeof...(K) != 0)
struct ArgState final {
  std::string_view value;
  std::array<const char*, N> choices;

  constexpr ArgState(std::array<const char*, N> choices)
    : choices(std::move(choices))
  {}
};

template<Id... K> requires (sizeof...(K) != 0)
struct ArgState<0, K...> final {
  std::string_view value;
};

template<Id... K>
constexpr auto Arg() -> ArgState<0, K...> { return {}; }

template<Id... K, size_t N>
constexpr auto Arg(const char* (&&choices)[N]) -> ArgState<N, K...> {
  return {std::to_array(choices)};
}

template<class T> struct IsArg: std::false_type {};
template<size_t N, Id... K> struct IsArg<ArgState<N, K...>>: std::true_type {};

template<class T> struct IsConstrainedArg: std::false_type {};
template<Id... K> struct IsConstrainedArg<ArgState<0, K...>>: std::false_type {};
template<size_t N, Id... K> struct IsConstrainedArg<ArgState<N, K...>>: std::true_type {};

}

namespace arp
{

template<size_t N, Id... K>
struct Meta<ArgState<N, K...>> final {
  static constexpr auto id() {
    return fmt::format("Arg<{}>", fmt::join(std::make_tuple(K.id()...), ", "));
  }

  static constexpr bool keyed_by(std::string_view key) {
    return (... || (key == K.id()));
  }

  template<Id X>
  static consteval bool keyed_by() {
    return (... || (X == K));
  }
};

}
