# arp

Stack-based command-line argument-parser for C++23.

## Nodes

The *arp* library structures command-line interfaces from compositions of nodes. The following nodes are provided.

* `Cmd<key>`: Subcommand
* `Pos<key>`: Positional argument
* `Arg<...key>`: Named argument
* `Opt<...key>`: Flag
* `Qty<...key>`: Counted flag
* `MutEx<...>`: Mutually exclusive group of `Arg`, `Opt`, `Qty`

## Example

```cpp
int main(int argc, const char** argv) {
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
    bool should_git_init = cmd.get<"git">().status;
  }
}
```

```sh
$ ./app new arp -gs23 -vvl
```

Given the interface and invocation above, we expect:

* `Cmd<new>` is invoked
* `Pos<name>` stores "arp"
* `Arg<std>` stores 23, which satisfies its value constraints: `[17, 20, 23, 26]`
* `Opt<git>` stores `true`
* `Qty<v>` stores 2, and
* `Opt<lib>` stores `true`

## Argument convention

The *arp* library supports the following argument conventions:

Single-dash arguments:
* `-kv`
* `-k v`
* `-k=v`

Double-dash arguments:
* `--key value`
* `--key=value`

Flags:
* `-a -b -c`
* `-a -bc`
* `-abc`

Flags may be combined with single-dash arguments:
* `-abckv`
* `-abck v`
* `-abck=v`

## Roadmap

The following features are presently unimplemented:

* Recursively generate usage/help from `Parser`
* Support required arguments as well as optional arguments
* Validate `MutEx` groups during parsing to produce errors if mutual-exclusion is violated
