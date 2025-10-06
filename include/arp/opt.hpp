#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <type_traits>

namespace arp
{

template<Id... K> requires (sizeof...(K) != 0)
struct OptState final {
  bool status = false;
};

template<Id... K>
constexpr auto Opt() -> OptState<K...> { return {}; }

template<class T> struct IsOpt: std::false_type {};
template<Id... K> struct IsOpt<OptState<K...>>: std::true_type {};

}

namespace arp
{

template<Id... K>
struct Meta<OptState<K...>> final {
  static constexpr auto id() {
    return fmt::format("Opt<{}>", fmt::join(std::make_tuple(K.id()...), ", "));
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
