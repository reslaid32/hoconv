
#include <argparser/argparser.hh>
#include <hoserial/hoserial.hh>
#include <hoserial/metadata.hh>
#include <view/vector_view.hh>

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace __detail
{

std::size_t
fsize (std::fstream &file)
{
  file.seekg (0, std::ios::end);
  std::size_t size = file.tellg ();
  file.seekg (0, std::ios::beg);
  return size;
}

std::vector<unsigned char>
bread (std::fstream &file, std::size_t size)
{
  std::vector<unsigned char> buffer;
  buffer.reserve (size);

  unsigned char byte;
  while (file.read (reinterpret_cast<char *> (&byte), sizeof (byte)))
    {
      buffer.push_back (byte);
      if (buffer.size () >= size)
        break;
    }

  return buffer;
}

struct argstate
{

  unsigned tabsize = 2;
  bool has_help = false;
  bool has_version = false;

  hoserial::Format input_format = hoserial::Format::YAML;
  hoserial::Format output_format = hoserial::Format::JSON;

  std::optional<std::string_view> file;
  std::optional<std::string_view> output_file;
};

__attribute__ ((noreturn)) void
throw_error (const char *_exec, const char *fmt, ...)

{
  va_list args;
  va_start (args, fmt);
  std::fprintf (stderr, "%s: ", _exec);
  std::fprintf (stderr, "\033[31merror\033[0m: ");
  std::vfprintf (stderr, fmt, args);
  va_end (args);
  std::exit (EXIT_FAILURE);
}

hoserial::Format
parse_format (const char *_exec, std::string_view &fmt)
{
  if (fmt == "json")
    return hoserial::Format::JSON;
  if (fmt == "yaml")
    return hoserial::Format::YAML;
  if (fmt == "toml")
    return hoserial::Format::TOML;
  throw_error (_exec, "unknown format: %s\n", fmt.data ());
}

void
parse (const char *_exec, const argparser::ArgParser &&parser, argstate &state)
{
  if (parser.has_flag ("--help") || parser.has_flag ("-h"))
    {
      state.has_help = true;
    }

  if (parser.has_flag ("--version") || parser.has_flag ("-v"))
    {
      state.has_version = true;
    }

  if (auto ts_opt = parser.get_option ("--tabsize"); ts_opt.has_value ())
    {
      state.tabsize = std::stoi (std::string (*ts_opt));
    }
  else if (auto t_opt = parser.get_option ("-t"); t_opt.has_value ())
    {
      state.tabsize = std::stoi (std::string (*t_opt));
    }

  auto out_opt = parser.get_option ("--output");
  if (!out_opt.has_value ())
    out_opt = parser.get_option ("-o");

  if (out_opt.has_value ())
    {
      state.output_file = *out_opt;
    }

  if (auto it_opt = parser.get_option ("--input-type"))
    state.input_format = parse_format (_exec, *it_opt);
  else if (auto it_opt_short = parser.get_option ("-it"))
    state.input_format = parse_format (_exec, *it_opt_short);

  if (auto ot_opt = parser.get_option ("--output-type"))
    state.output_format = parse_format (_exec, *ot_opt);
  else if (auto ot_opt_short = parser.get_option ("-ot"))
    state.output_format = parse_format (_exec, *ot_opt_short);

  auto file_opt = parser.get_option ("--file");
  if (!file_opt.has_value ())
    file_opt = parser.get_option ("-f");

  if (!file_opt.has_value ())
    {
      auto pos_args = parser.positional_args ();
      if (!pos_args.empty ())
        {
          file_opt = pos_args[0];
        }
    }

  if (file_opt.has_value ())
    {
      state.file = *file_opt;
    }
}

}

int
main (int argc, char *argv[])
{
  view::vector_view<char *> args (argv, argv + argc);
  argparser::ArgParser parser (args);

  __detail::argstate state;
  __detail::parse (argv[0], std::move (parser), state);

  if (state.has_version)
    {
      std::printf ("\033[1mhoserial version\033[0m \033[32m%s\033[0m\n",
                   hoserial::version.to_string ().c_str ());
      return 0;
    }

  if (state.has_help)
    {
      std::printf ("\033[1mUsage:\033[0m %s [options] [file]\n\n", args[0]);
      std::puts (
          "Format converter, supports types: "
          "{ \033[33mtoml\033[0m \033[34myaml\033[0m \033[32mjson\033[0m }\n");
      std::puts ("Options:");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-h, --help",
                   "Show this help message");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-v, --version",
                   "Show version information");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-t, --tabsize",
                   "Output tab size (0 = compact, 1+ = pretty), by default 2");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-f, --file",
                   "Input file path");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-it, --input-type",
                   "Input file type");
      std::printf ("  \033[33m%-20s\033[0m %s\n", "-ot, --output-type",
                   "Output file type");
      return 0;
    }

  if (state.file)
    {
      std::fstream file (std::string (*state.file),
                         std::ios::in | std::ios::binary);
      if (!file)
        __detail::throw_error (argv[0], "could not open file '%s'\n",
                               state.file->data ());

      std::vector<unsigned char> buffer
          = __detail::bread (file, __detail::fsize (file));

      if (buffer.empty ())
        __detail::throw_error (argv[0], "file '%s' is empty\n",
                               state.file->data ());

      hoserial::ParserFactory parser_factory;
      hoserial::DumperFactory dumper_factory;
      hoserial::ConvertFactory converter_factory;

      auto parser = parser_factory.create_parser (state.input_format);
      auto node = parser->parse (std::string_view (
          reinterpret_cast<const char *> (buffer.data ()), buffer.size ()));
      auto dumper = dumper_factory.create_dumper (state.output_format);
      auto converted_node
          = converter_factory.convert (state.output_format, node);
      if (state.output_file)
        {
          std::ofstream outfile (std::string (*state.output_file),
                                 std::ios::binary);
          if (!outfile)
            __detail::throw_error (argv[0],
                                   "could not open output file '%s'\n",
                                   state.output_file->data ());
          dumper->dump (outfile, converted_node, state.tabsize);
        }
      else
        {
          dumper->dump (std::cout, converted_node, state.tabsize);
        }
    }
  else
    {
      __detail::throw_error (argv[0], "no file specified\n");
      return 1;
    }

  return 0;
}
