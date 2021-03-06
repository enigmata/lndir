#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

#include "version.hpp"

namespace fs = std::filesystem;

constexpr std::string_view usage(
    "\n"
    "Name:\n"
    "\n"
    "lndir - create shadow directory of symlinks to another directory tree\n"
    "\n"
    "Usage:\n"
    "\n"
    "lndir [options] from-dir [to-dir]\n"
    "\n"
    "Description:\n"
    "\n"
    "The lndir program makes a shadow copy <todir> of a directory tree\n"
    "<fromdir>, except that the shadow is not populated with real files\n"
    "but instead with symbolic links pointing at the real files in the\n"
    "<fromdir> directory tree.\n"
    "\n"
    "When <todir> is not specified, it defaults to the current directory,\n"
    "from which lndir is run.\n"
    "\n"
    "Options:\n"
    "\n"
    "--suffix <suffix>\n"
    "     Append the text <suffix> to each link in the <to-dir>.\n"
    "     For example, given \"--suffix -v7\", the file \"from-dir/foo\"\n"
    "     will be linked as \"<to-dir>/foo-v7\".\n"
    "--version\n"
    "     Print the tool's semver version as <major>.<minor>.<patch>\n"
    "--help\n"
    "     Display this usage help.\n\n");

struct Link_parms {
  fs::path from_dir, to_dir, filename_suffix;
};

enum class Cmd_options { none, help, version };

struct Cmd_args {
  Link_parms link_parms;
  Cmd_options options;
};

bool is_valid_directory(fs::path dir, std::error_code &ec) noexcept {
  if (!fs::exists(dir, ec) || !fs::is_directory(fs::status(dir, ec))) {
    std::cout << "Error: " << dir << " is not a valid directory!\n";
    return false;
  }
  return true;
}

void normalize_path(fs::path &path) {
  path = path.lexically_normal();
  if (path.string().back() == fs::path::preferred_separator) {
    std::string path_string = path;
    path_string.pop_back();
    path = path_string;
  }
  return;
}

std::optional<Cmd_args> parse_args(int argc, char *argv[]) noexcept {
  Cmd_args args = Cmd_args();

  for (int arg_num = 1; arg_num < argc; ++arg_num) {
    std::string_view arg_string{argv[arg_num]};
    if (arg_string == "--help") {
      args.options = Cmd_options::help;
      return args;
    }
    if (arg_string == "--version") {
      args.options = Cmd_options::version;
      return args;
    }
    if (arg_string == "--suffix") {
      if (!args.link_parms.filename_suffix.empty()) {
        std::cout << "\nError: --suffix option specified more than once.\n";
        return std::nullopt;
      }
      ++arg_num;
      if (arg_num >= argc) {
        std::cout << "\nError: No text provided for the --suffix option.\n";
        return std::nullopt;
      }
      args.link_parms.filename_suffix = argv[arg_num];
    } else {
      if (args.link_parms.from_dir.empty()) {
        args.link_parms.from_dir = argv[arg_num];
      } else if (args.link_parms.to_dir.empty()) {
        args.link_parms.to_dir = argv[arg_num];
      }
    }
  }

  if (args.link_parms.from_dir.empty()) {
    std::cout << "\nError: Missing <from_dir>, a mandatory argument.\n";
    return std::nullopt;
  }

  if (args.link_parms.to_dir.empty()) {
    args.link_parms.to_dir = fs::current_path();
  }

  std::error_code ec;
  if (!is_valid_directory(args.link_parms.from_dir, ec) ||
      !is_valid_directory(args.link_parms.to_dir, ec)) {
    return std::nullopt;
  }

  if (fs::equivalent(args.link_parms.from_dir, args.link_parms.to_dir, ec)) {
    std::cout << "\nError: from-dir and to-dir are the same directory!\n";
    return std::nullopt;
  }

  normalize_path(args.link_parms.from_dir);
  normalize_path(args.link_parms.to_dir);
  normalize_path(args.link_parms.filename_suffix);

  return args;
}

void link_dir_trees(Link_parms link_parms, int indent = 4) noexcept {
  std::error_code ec;
  std::string::size_type from_dir_len =
      link_parms.from_dir.string().length() + 1;
  fs::path to_path{""};
  bool must_add_suffix = !link_parms.filename_suffix.empty();

  for (auto i = fs::recursive_directory_iterator(link_parms.from_dir);
       i != fs::recursive_directory_iterator(); ++i) {
    to_path = link_parms.to_dir / i->path().string().substr(from_dir_len);
    if (i->is_directory(ec)) {
      fs::create_directory(to_path, link_parms.to_dir, ec);
      std::cout << std::string(indent + ((i.depth() + 1) * 2), ' ')
                << i->path().filename() << "\n";
    } else {
      if (must_add_suffix) {
        if (to_path.has_extension()) {
          fs::path ext = to_path.extension();
          to_path = to_path.parent_path() / to_path.stem();
          to_path += link_parms.filename_suffix;
          to_path += ext;
        } else {
          to_path += link_parms.filename_suffix;
        }
      }
      fs::create_symlink(i->path(), to_path, ec);
    }
  }
  return;
}

int main(int argc, char *argv[]) {

  std::optional<Cmd_args> cmd_args = parse_args(argc, argv);

  if (!cmd_args || cmd_args->options == Cmd_options::help) {
    std::cout << usage;
    return -1;
  }
  if (cmd_args->options == Cmd_options::version) {
    std::cout << "lndir " << VERSION_MAJOR << "." << VERSION_MINOR << "."
              << VERSION_PATCH << "\n";
    return 0;
  }

  std::cout << "Linking:\n";
  std::cout << "  from dir: " << cmd_args->link_parms.from_dir << "\n";
  std::cout << "  to dir:   " << cmd_args->link_parms.to_dir << "\n";
  if (!cmd_args->link_parms.filename_suffix.empty()) {
    std::cout << "  suffix:   " << cmd_args->link_parms.filename_suffix << "\n";
  }
  std::cout << "  directories linked:\n";
  std::cout << "    " << cmd_args->link_parms.from_dir.filename() << "\n";

  link_dir_trees(cmd_args->link_parms);

  return 0;
}
