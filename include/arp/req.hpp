#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>

namespace arp
{

template<class T>
struct Req: T {
  using T::T;
  using Type = T;
};

template<class T> struct IsReq: std::false_type {};
template<class T> struct IsReq<Req<T>>: std::true_type {};

}

namespace arp
{

template<class T>
struct Meta<Req<T>> final {
  static constexpr auto id() {
    return fmt::format("Req<{}>", Meta<T>::id());
  }

  static constexpr bool keyed_by(std::string_view key) {
    return Meta<T>::keyed_by(key);
  }

  template<Id X>
  static consteval bool keyed_by() {
    return Meta<T>::template keyed_by<X>();
  }
};

}
