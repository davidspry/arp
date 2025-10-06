#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <cstddef>
#include <type_traits>

namespace arp
{

template<Id... K> requires (sizeof...(K) != 0)
struct QtyState final {
  size_t count = 0;
};

template<Id... K>
constexpr auto Qty() -> QtyState<K...> { return {}; }

template<class T> struct IsQty: std::false_type {};
template<Id... K> struct IsQty<QtyState<K...>>: std::true_type {};

}

namespace arp
{

template<Id... K>
struct Meta<QtyState<K...>> final {
  static constexpr auto id() {
    return fmt::format("Qty<{}>", fmt::join(std::make_tuple(K.id()...), ", "));
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
