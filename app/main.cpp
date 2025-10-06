#include <arp/arp.hpp>

#include <fmt/base.h>

auto main(int argc, const char** argv) -> int {
  using namespace arp;

  auto parser = Parser{
    Cmd<"new">(Parser{
      Pos<"name">(),
      Arg<'s', "std">({"17", "20", "23", "26"}),
      Opt<'g', "git">(),
      Qty<'v'>(),
      MutEx{
        Opt<'x', "exe">(),
        Opt<'l', "lib">(),
        Opt<'m', "mod">(),
      },
    })
  };

  if (auto err = parser.parse(argc, argv))
    fmt::println("error: '{}'", err->msg);

  if (const auto& cmd = parser.get<"new">()) {
    fmt::println("name={} std={} git={} exe={} lib={} mod={} verbose={}",
      cmd.get<"name">().value,
      cmd.get<"std">().value,
      cmd.get<"git">().status,
      cmd.get<'x'>().status,
      cmd.get<'l'>().status,
      cmd.get<'m'>().status,
      cmd.get<'v'>().count);
  }
}
