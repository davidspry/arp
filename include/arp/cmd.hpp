#pragma once

#include <arp/id.hpp>
#include <arp/meta.hpp>

#include <fmt/format.h>

#include <utility>

namespace arp
{

template<class... T>
struct Parser;

template<Id K, class... T>
struct CmdState final {
  Parser<T...> parser;
  bool invoked = false;

  constexpr CmdState(Parser<T...>&& parser)
    : parser(std::move(parser))
  {}

  constexpr operator bool() const {
    return invoked;
  }

  template<Id X, class Self>
  constexpr auto&& get(this Self&& self) {
    return std::forward<Self>(self).parser.template get<X>();
  }
};

template<Id K, class... T>
constexpr auto Cmd(Parser<T...>&& parser) -> CmdState<K, T...> {
  return {std::move(parser)};
}

template<class T> struct IsCmd: std::false_type {};
template<Id K, class... T> struct IsCmd<CmdState<K, T...>>: std::true_type {};

}

namespace arp
{

template<Id K, class... T>
struct Meta<CmdState<K, T...>> final {
  static constexpr auto id() {
    return fmt::format("Cmd<{}>", K.id());
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
