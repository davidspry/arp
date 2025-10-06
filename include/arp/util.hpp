#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

namespace arp
{

template<size_t N, class F>
constexpr void template_for(F&& fn) {
  [&fn]<size_t... K>(std::index_sequence<K...>) {
    (..., std::forward<F>(fn).template operator()<K>());
  }(std::make_index_sequence<N>());
}

template<class... T, class F>
  requires requires { (std::declval<F>().operator()(std::declval<T&>()), ...); }
constexpr void template_for(std::tuple<T...>& tuple, F&& fn) {
  std::apply([&](auto&... x) { (..., std::forward<F>(fn)(x)); }, tuple);
}

template<class... T, class F>
  requires requires { (std::declval<F>().operator()(std::declval<T&>()), ...); }
constexpr void template_for(const std::tuple<T...>& tuple, F&& fn) {
  std::apply([&](auto&... x) { (..., std::forward<F>(fn)(x)); }, tuple);
}

}
