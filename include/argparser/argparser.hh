#pragma once

#include "../view/vector_view.hh"

#include <optional>
#include <string_view>
#include <vector>

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

  std::vector<char *>
  positional_args () const
  {
    std::vector<char *> positions;
    for (size_t i = 1; i < args_.size (); ++i)
      {
        if (args_[i][0] == '-')
          {
            if (i + 1 < args_.size () && args_[i + 1][0] != '-')
              ++i;
          }
        else
          {
            positions.push_back (args_[i]);
          }
      }
    return positions;
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
