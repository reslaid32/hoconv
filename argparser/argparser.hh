#pragma once

#include "../view/vector_view.hh"

#include <optional>
#include <string_view>

namespace argparser
{

class ArgParser
{
public:
  using view_type = view::vector_view<char *>;
  using string_view = std::string_view;

  ArgParser (int argc, char **argv) : args_ (argv, argc) {}

  ArgParser (view_type args) : args_ (args) {}

  bool
  has_flag (string_view flag) const
  {
    for (size_t i = 1; i < args_.size (); ++i)
      {
        if (flag == args_[i])
          return true;
      }
    return false;
  }

  std::optional<string_view>
  get_option (string_view option) const
  {
    for (size_t i = 1; i + 1 < args_.size (); ++i)
      {
        if (option == args_[i])
          return string_view (args_[i + 1]);
      }
    return std::nullopt;
  }

  view_type
  positional_args () const
  {
    size_t start = 1;
    while (start < args_.size () && args_[start][0] == '-')
      ++start;
    return view_type (args_.data () + start, args_.size () - start);
  }

  view_type
  raw () const
  {
    return args_;
  }

private:
  view_type args_;
};

}
