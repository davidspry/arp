#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>

namespace arp
{

template<Id K>
struct PosState final {
  std::string_view value;
};

template<Id K>
constexpr auto Pos() -> PosState<K> { return {}; }

template<class T> struct IsPos: std::false_type {};
template<Id K> struct IsPos<PosState<K>>: std::true_type {};

}

namespace arp
{

template<Id K>
struct Meta<PosState<K>> final {
  static constexpr auto id() {
    return fmt::format("Pos<{}>", K.id());
  }

  static constexpr bool keyed_by(std::string_view key) {
    return key == K.id();
  }

  template<Id X>
  static consteval bool keyed_by() {
    return X == K;
  }
};

}
