#pragma once

#include <arp/arg.hpp>
#include <arp/cmd.hpp>
#include <arp/id.hpp>
#include <arp/meta.hpp>
#include <arp/mutex.hpp>
#include <arp/opt.hpp>
#include <arp/pos.hpp>
#include <arp/qty.hpp>
#include <arp/req.hpp>
#include <arp/tokens.hpp>
#include <arp/util.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <bitset>
#include <functional>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace arp
{

struct ParserError {
  enum Enum {
    invalid_argc,
    missing_value,
    mutex_violation,
    unknown_key,
    unknown_pos,
    unknown_value,
  };

  Enum err;
  std::string msg;
};

template<class... T>
struct Parser final {
  std::tuple<T...> m_nodes;
  std::bitset<sizeof...(T)> m_parsed;

  Parser(T&&... nodes)
    : m_nodes(std::forward_as_tuple(std::forward<T>(nodes)...))
  {}

  /// Parse an array of tokenised arguments
  std::optional<ParserError> parse(std::span<const char* const> args);

  /// Parse a program's 'main' args, including the executable
  /// path in the first position.
  std::optional<ParserError> parse(int argc, const char** argv);

  /// Obtain the node keyed by K from the parser
  template<Id K, class Self> requires (... || Meta<T>::template keyed_by<K>())
  constexpr auto&& get(this Self&&);

private:
  template<Id K> requires (... || Meta<T>::template keyed_by<K>())
  static consteval size_t index();

  std::optional<ParserError> parse_double_type(std::string_view token, std::span<const char* const>&);
  std::optional<ParserError> parse_single_type(std::string_view token, std::span<const char* const>&);
  std::optional<ParserError> parse_cmd_or_pos(std::string_view token, std::span<const char* const>&);
  std::optional<ParserError> parse_pos(std::string_view token, std::span<const char* const>&);

  template<size_t K, class Node> requires (IsOpt<Node>::value || IsQty<Node>::value)
  std::optional<ParserError> process_node(Node&);

  template<size_t K, class Node> requires (IsArg<Node>::value)
  std::optional<ParserError> process_node(Node&, std::string_view value);

  template<size_t K, class Node, class F>
    requires std::is_invocable_r_v<std::optional<std::string_view>, F>
  std::optional<ParserError> dispatch_node(Node&, std::string_view key, F&& consume_value);
};

template<class... T>
std::optional<ParserError> Parser<T...>::parse(std::span<const char* const> args) {
  bool parsing_opts = true;

  while (!args.empty()) {
    std::string_view arg = args.front();

    if (arg.empty())
      std::ignore = consume_token(args);

    if (!parsing_opts) {
      if (auto key = consume_token(args);
          auto err = parse_pos(key, args))
        return *err;
      continue;
    }

    if (try_consume_token("--", args)) {
      parsing_opts = false;
      continue;
    }

    if (arg.size() > 2 && arg.starts_with("--")) {
      if (auto key = consume_token(args);
          auto err = parse_double_type(key, args))
        return *err;
      continue;
    }

    if (arg.size() > 1 && arg.starts_with('-')) {
      if (auto key = consume_token(args);
          auto err = parse_single_type(key, args))
        return *err;
      continue;
    }

    if (auto key = consume_token(args);
        auto err = parse_cmd_or_pos(key, args)) {
      return *err;
    }
  }

  // validate_requirements();
  // validate_mutex_groups();

  return std::nullopt;
}

template<class... T>
std::optional<ParserError> Parser<T...>::parse(int argc, const char** argv) {
  if (argc < 1)
    return ParserError{
      .err = ParserError::invalid_argc,
      .msg = fmt::format("argc is {}", !argc ? "zero" : "negative")
    };

  return parse({argv + 1, static_cast<size_t>(argc - 1)});
}

template<class... T>
template<Id K, class Self> requires (... || Meta<T>::template keyed_by<K>())
constexpr auto&& Parser<T...>::get(this Self&& self) {
  constexpr size_t X = Parser<T...>::index<K>();

  using Node = std::tuple_element_t<X, std::tuple<T...>>;
  auto& node = std::get<X>(std::forward<Self>(self).m_nodes);

  if constexpr (IsMutEx<Node>::value) {
    constexpr size_t M = [&] {
      size_t index = 0, found = 0;

      template_for(node.group, [&]<class MutExNode>(MutExNode&) {
        if constexpr (Meta<std::remove_cvref_t<MutExNode>>::template keyed_by<K>())
          found = true;

        if (!found)
          index++;
      });

      return index;
    }();

    return std::get<M>(node.group);
  }

  if constexpr (!IsMutEx<Node>::value)
    return node;
}

template<class... T>
template<Id K> requires (... || Meta<T>::template keyed_by<K>())
consteval size_t Parser<T...>::index() {
  size_t index = 0;

  template_for<sizeof...(T)>([&]<size_t X>() {
    using Node = std::tuple_element_t<X, std::tuple<T...>>;

    if constexpr (Meta<Node>::template keyed_by<K>())
      index = X;
  });

  return index;
}

template<class... T>
auto Parser<T...>::parse_double_type(std::string_view token, std::span<const char* const>& args) -> std::optional<ParserError> {
  std::string_view key = token.substr(2);
  std::optional<std::string_view> val;

  if (auto k = key.find('='); k != std::string_view::npos) {
    val = key.substr(k + 1);
    key = key.substr(0, k);
  }

  bool match = false;
  std::optional<ParserError> error;

  template_for<sizeof...(T)>([&, this]<size_t K> -> std::optional<ParserError> {
    using Node = std::tuple_element_t<K, std::tuple<T...>>;
    Node& node = std::get<K>(m_nodes);

    if (match || error)
      return std::nullopt;

    if (match = Meta<Node>::keyed_by(key); !match)
      return std::nullopt;

    return error = dispatch_node<K>(node, key, [&] {
      return val ? *val : try_consume_token(args);
    });
  });

  if (error)
    return *error;

  if (!match)
    return ParserError{
      .err = ParserError::unknown_key,
      .msg = fmt::format("unknown key: {}", key)
    };

  return std::nullopt;
}

template<class... T>
auto Parser<T...>::parse_single_type(std::string_view token, std::span<const char* const>& args) -> std::optional<ParserError> {
  std::string_view keys = token.substr(1);
  bool value_consumed = false;

  while (!value_consumed && !keys.empty()) {
    std::string_view key = keys.substr(0, 1);
    std::optional<std::string_view> val;
    std::optional<ParserError> error;
    bool match = false;

    if (auto rem = keys.substr(1); !rem.empty()) {
      if (rem.starts_with('='))
        rem = keys.substr(2);

      if (!rem.empty())
        val = rem;
    }

    template_for<sizeof...(T)>([&, this]<size_t K> -> std::optional<ParserError> {
      using Node = std::tuple_element_t<K, std::tuple<T...>>;
      Node& node = std::get<K>(m_nodes);

      if (match || error)
        return std::nullopt;

      if (match = Meta<Node>::keyed_by(key); !match)
        return std::nullopt;

      return error = dispatch_node<K>(node, key, [&] {
        return value_consumed = true, val ? *val : try_consume_token(args);
      });
    });

    if (error)
      return *error;

    if (!match)
      return ParserError{
        .err = ParserError::unknown_key,
        .msg = fmt::format("unknown key: {}", key)
      };

    keys.remove_prefix(1);
  }

  return std::nullopt;
}

template<class... T>
auto Parser<T...>::parse_cmd_or_pos(std::string_view token, std::span<const char* const>& args) -> std::optional<ParserError> {
  std::string_view key = token;
  std::optional<ParserError> error;
  bool match = false;

  template_for<sizeof...(T)>([&, this]<size_t K> -> std::optional<ParserError> {
    using Node = std::tuple_element_t<K, std::tuple<T...>>;
    Node& node = std::get<K>(m_nodes);

    if (match || error)
      return std::nullopt;

    if (match = Meta<Node>::keyed_by(key); !match)
      return std::nullopt;

    if constexpr (IsCmd<Node>::value) {
      if (error = node.parser.parse(consume_all(args)); !error) {
        node.invoked = true;
        m_parsed[K] = true;
      }
    }

    return error;
  });

  if (error)
    return *error;

  if (match)
    return std::nullopt;

  return parse_pos(token, args);
}

template<class... T>
auto Parser<T...>::parse_pos(std::string_view token, std::span<const char* const>& args) -> std::optional<ParserError> {
  bool match = false;

  template_for<sizeof...(T)>([&, this]<size_t K> {
    using Node = std::tuple_element_t<K, std::tuple<T...>>;
    Node& node = std::get<K>(m_nodes);

    if (match)
      return;

    if constexpr (IsPos<Node>::value) {
      if (!m_parsed[K]) {
        match = true;
        m_parsed[K] = true;
        node.value = token;
      }
    }
  });

  if (!match)
    return ParserError{
      .err = ParserError::unknown_pos,
      .msg = fmt::format("unknown positional argument: {}", token)
    };

  return std::nullopt;
}

template<class... T>
template<size_t K, class Node> requires (IsOpt<Node>::value || IsQty<Node>::value)
auto Parser<T...>::process_node(Node& node) -> std::optional<ParserError> {
  m_parsed[K] = true;

  if constexpr (IsOpt<Node>::value)
    node.status = true;

  if constexpr (IsQty<Node>::value)
    node.count += 1;

  return std::nullopt;
}

template<class... T>
template<size_t K, class Node> requires (IsArg<Node>::value)
auto Parser<T...>::process_node(Node& node, std::string_view value) -> std::optional<ParserError> {
  m_parsed[K] = true;

  if constexpr (IsConstrainedArg<Node>::value) {
    if (!std::ranges::contains(node.choices, value))
      return ParserError{
        .err = ParserError::unknown_value,
        .msg = fmt::format("value '{}' not in choices list: {}", value, node.choices)
      };
  }

  node.value = value;

  return std::nullopt;
}

template<class... T>
template<size_t K, class Node, class F>
  requires std::is_invocable_r_v<std::optional<std::string_view>, F>
auto Parser<T...>::dispatch_node(Node& node, std::string_view key, F&& consume_value) -> std::optional<ParserError> {
  if constexpr (IsMutEx<Node>::value) {
    std::optional<ParserError> error;

    template_for(node.group, [&]<class MutExNode>(MutExNode& mutex_node) -> std::optional<ParserError> {
      if (error)
        return std::nullopt;

      return Meta<std::remove_cvref_t<MutExNode>>::keyed_by(key)
        ? error = dispatch_node<K>(mutex_node, key, std::forward<F>(consume_value))
        : std::nullopt;
    });

    return error;
  }

  if constexpr (IsOpt<Node>::value || IsQty<Node>::value)
    return process_node<K>(node);

  if constexpr (IsArg<Node>::value) {
    auto value = std::invoke(consume_value);

    if (!value)
      return ParserError{
        .err = ParserError::missing_value,
        .msg = fmt::format("value not supplied for arg '{}'", key)
      };

    return process_node<K>(node, *value);
  }

  throw std::runtime_error("unhandled node");
}

}
