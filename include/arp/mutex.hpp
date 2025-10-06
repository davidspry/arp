#pragma once

#include <arp/arg.hpp>
#include <arp/id.hpp>
#include <arp/meta.hpp>
#include <arp/util.hpp>
#include <arp/opt.hpp>
#include <arp/qty.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <tuple>
#include <type_traits>

namespace arp
{

template<class... T> requires (sizeof...(T) != 0 && (... || (IsOpt<T>::value || IsQty<T>::value || IsArg<T>::value)))
struct MutEx final {
  std::tuple<T...> group;

  constexpr MutEx(T&&... group)
    : group(std::forward_as_tuple(std::forward<T>(group)...))
  {}
};

template<class... T> struct IsMutEx: std::false_type {};
template<class... T> struct IsMutEx<MutEx<T...>>: std::true_type {};

}

namespace arp
{

template<class... T>
struct Meta<MutEx<T...>> final {
  static constexpr auto id() {
    return fmt::format("MutEx<{}>", fmt::join(std::make_tuple(Meta<T>::id()...), ", "));
  }

  static constexpr bool keyed_by(std::string_view key) {
    return (... || Meta<T>::keyed_by(key));
  }

  template<Id X>
  static consteval bool keyed_by() {
    return (... || Meta<T>::template keyed_by<X>());
  }
};

}
