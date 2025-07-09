
#include "hoserial/version_container.hh"
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
#include <vector>

namespace __detail
{

struct ArgState
{

  unsigned tabsize = 2;
  bool has_help = false;
  bool has_version = false;

  hoserial::Format input_format = hoserial::Format::JSON;
  hoserial::Format output_format = hoserial::Format::YAML;

  std::optional<std::string_view> file;
  std::optional<std::string_view> output_file;
};

struct Driver
{

  const char* exec;
  view::vector_view<char*> args;
  hoserial::VersionContainer ver;
  ArgState argstate;

  Driver(const char* exec, view::vector_view<char*>&& args,
         hoserial::VersionContainer&& ver)
      : exec(exec), args(args), ver(ver)
  {
  }

  virtual ~Driver() = default;
  virtual int entry() = 0;
};

__attribute__((noreturn)) void
__exit(Driver& drv, int code, const char* hint)
{
  if (hint)
    std::fprintf(stderr, "[%s] exit(%d) hint: \"%s\"\n", drv.exec, code, hint);
  exit(code);
}

__attribute__((noreturn)) void
__throw_error(Driver& drv, const char* fmt, ...)

{
  va_list args;
  va_start(args, fmt);
  std::fprintf(stderr, "%s: ", drv.exec);
  std::fprintf(stderr, "\033[31merror\033[0m: ");
  std::vfprintf(stderr, fmt, args);
  va_end(args);
  __exit(drv, EXIT_FAILURE, NULL);
}

hoserial::Format
__parse_format(Driver& drv, std::string_view& fmt)
{
  if (fmt == "json")
    return hoserial::Format::JSON;
  if (fmt == "yaml")
    return hoserial::Format::YAML;
  if (fmt == "toml")
    return hoserial::Format::TOML;
  __throw_error(drv, "unknown format: %s\n", fmt.data());
}

void
__parse_args(Driver& drv, const argparser::ArgParser&& parser)
{
  if (parser.has_flag("--help") || parser.has_flag("-h"))
  {
    drv.argstate.has_help = true;
  }

  if (parser.has_flag("--version") || parser.has_flag("-v"))
  {
    drv.argstate.has_version = true;
  }

  if (auto ts_opt = parser.get_option("--tabsize"); ts_opt.has_value())
  {
    drv.argstate.tabsize = std::stoi(std::string(*ts_opt));
  }
  else if (auto t_opt = parser.get_option("-t"); t_opt.has_value())
  {
    drv.argstate.tabsize = std::stoi(std::string(*t_opt));
  }

  auto out_opt = parser.get_option("--output");
  if (!out_opt.has_value())
    out_opt = parser.get_option("-o");

  if (out_opt.has_value())
  {
    drv.argstate.output_file = *out_opt;
  }

  if (auto it_opt = parser.get_option("--input-type"))
    drv.argstate.input_format = __parse_format(drv, *it_opt);
  else if (auto it_opt_short = parser.get_option("-it"))
    drv.argstate.input_format = __parse_format(drv, *it_opt_short);

  if (auto ot_opt = parser.get_option("--output-type"))
    drv.argstate.output_format = __parse_format(drv, *ot_opt);
  else if (auto ot_opt_short = parser.get_option("-ot"))
    drv.argstate.output_format = __parse_format(drv, *ot_opt_short);

  auto file_opt = parser.get_option("--file");
  if (!file_opt.has_value())
    file_opt = parser.get_option("-f");

  if (!file_opt.has_value())
  {
    auto pos_args = parser.positional_args();
    if (!pos_args.empty())
    {
      file_opt = pos_args[0];
    }
  }

  if (file_opt.has_value())
  {
    drv.argstate.file = *file_opt;
  }
}

std::size_t
__file_size(std::fstream& file)
{
  file.seekg(0, std::ios::end);
  std::size_t size = file.tellg();
  file.seekg(0, std::ios::beg);
  return size;
}

std::vector<unsigned char>
__read(std::istream& input)
{
  std::vector<unsigned char> buffer;
  unsigned char byte;
  while (input.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
    buffer.push_back(byte);
  return buffer;
}

std::vector<unsigned char>
__read(std::fstream& file, std::size_t size)
{
  std::vector<unsigned char> buffer;
  buffer.reserve(size);

  unsigned char byte;
  while (file.read(reinterpret_cast<char*>(&byte), sizeof(byte)))
  {
    buffer.push_back(byte);
    if (buffer.size() >= size)
      break;
  }

  return buffer;
}

}

struct HoConv2_Driver : public __detail::Driver
{
  HoConv2_Driver(const char* exec, view::vector_view<char*>&& args)
      : Driver(exec, std::move(args), { 2, 0, 0 })
  {
  }

  void
  emit_version([[maybe_unused]] Driver& drv)
  {
    std::printf("\033[1mdriver   version\033[0m \033[32m%s\033[0m\n",
                ver.to_string().c_str());
    std::printf("\033[1mhoserial version\033[0m \033[32m%s\033[0m\n",
                hoserial::version.to_string().c_str());
  }

  void
  emit_help(Driver& drv)
  {
    std::printf("\033[1mUsage:\033[0m %s [options] [file]\n\n", drv.exec);
    std::puts(
        "Format converter, supports types: "
        "{ \033[33mtoml\033[0m \033[34myaml\033[0m \033[32mjson\033[0m }\n");
    std::puts("Options:");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-h, --help",
                "Show this help message");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-v, --version",
                "Show version information");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-t, --tabsize",
                "Output tab size (0 = compact, 1+ = pretty), by default 2");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-f, --file",
                "Input file path   (optional, defaults to stdin)");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-it, --input-type",
                "Input file type");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-o, --output",
                "Output file path  (optional, defaults to stdout)");
    std::printf("  \033[33m%-20s\033[0m %s\n", "-ot, --output-type",
                "Output file type");
  }

  int
  entry() override
  {
    argparser::ArgParser ap(args);
    __detail::__parse_args(*this, std::move(ap));

    if (argstate.has_version)
    {
      emit_version(*this);
      return 0;
    }

    if (argstate.has_help)
    {
      emit_help(*this);
      return 0;
    }

    std::vector<unsigned char> buffer;

    if (argstate.file)
    {
      std::fstream file(std::string(*argstate.file),
                        std::ios::in | std::ios::binary);
      if (!file)
        __detail::__throw_error(*this, "could not open file '%s'\n",
                                argstate.file->data());

      buffer = __detail::__read(file, __detail::__file_size(file));
    }
    else
    {
      if (std::cin.peek() == EOF)
        __detail::__throw_error(*this, "no input provided on stdin\n");

      buffer = __detail::__read(std::cin);
    }

    if (buffer.empty())
      __detail::__throw_error(*this, "input is empty\n");

    hoserial::ParserFactory parser_factory;
    hoserial::DumperFactory dumper_factory;
    hoserial::ConvertFactory converter_factory;

    auto parser = parser_factory.create_parser(argstate.input_format);
    auto node = parser->parse(std::string_view(
        reinterpret_cast<const char*>(buffer.data()), buffer.size()));
    auto dumper = dumper_factory.create_dumper(argstate.output_format);
    auto converted_node = converter_factory.convert(argstate.output_format, node);

    if (argstate.output_file)
    {
      std::ofstream outfile(std::string(*argstate.output_file), std::ios::binary);
      if (!outfile)
        __detail::__throw_error(*this, "could not open output file '%s'\n",
                                argstate.output_file->data());
      dumper->dump(outfile, converted_node, argstate.tabsize);
    }
    else
    {
      dumper->dump(std::cout, converted_node, argstate.tabsize);
    }

    return 0;
  }
};

int
main(int argc, char* argv[])
{
  view::vector_view<char*> args(argv, argv + argc);
  HoConv2_Driver drv(argv[0], std::move(args));
  return drv.entry();
}
