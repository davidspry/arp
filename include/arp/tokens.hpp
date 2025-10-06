#pragma once

#include <cassert>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>

namespace arp
{

constexpr std::span<const char* const> consume_all(std::span<const char* const>& span) {
  auto copy = span;
  span = {};
  return copy;
}

constexpr std::string_view consume_token(std::span<const char* const>& span) {
  if (span.empty())
    throw std::runtime_error("cannot consume from empty span");

  std::string_view token = span.front();
  span = span.subspan(1);
  return token;
}

constexpr std::optional<std::string_view> try_consume_token(std::span<const char* const>& span) {
  if (span.empty())
    return std::nullopt;

  return consume_token(span);
}

constexpr std::optional<std::string_view> try_consume_token(std::string_view token, std::span<const char* const>& span) {
  if (!span.empty() && span.front() == token)
    return consume_token(span);

  return std::nullopt;
}

}
