#include <filesystem>
#include <iostream>
#include <optional>
#include <string_view>

namespace fs = std::filesystem;

constexpr std::string_view usage("\n\
Name:\n\
lndir - create a shadow directory of symbolic links to another directory tree\n\
\n\
Usage:\n\
lndir from-dir [to-dir]\n\
\n\
Description:\n\
The lndir program makes a shadow copy <todir> of a directory tree <fromdir>,\n\
except that the shadow is not populated with real files but instead with\n\
symbolic links pointing at the real files in the <fromdir> directory tree.\n\
\n\
When <todir> is not specified, it defaults to the current directory, from\n\
which lndir is run.\n");

struct Link_parms {
  fs::path from_dir, to_dir;
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

std::optional<Link_parms> parse_args(int argc, char *argv[]) noexcept {
  if (argc < 2) {
    std::cout << "Error: At least one argument is required\n";
    return std::nullopt;
  }

  Link_parms parms{argv[1], fs::current_path()};
  if (argc == 3) {
    parms.to_dir = argv[2];
  }

  std::error_code ec;
  if (!is_valid_directory(parms.from_dir, ec) ||
      !is_valid_directory(parms.to_dir, ec)) {
    return std::nullopt;
  }

  if (fs::equivalent(parms.from_dir, parms.to_dir, ec)) {
    std::cout << "Error: from-dir and to-dir are the same directory!\n";
    return std::nullopt;
  }

  normalize_path(parms.from_dir);
  normalize_path(parms.to_dir);

  return parms;
}

void link_dir_trees(Link_parms link_parms, int indent = 4) noexcept {
  std::error_code ec;
  std::string::size_type from_dir_len =
      link_parms.from_dir.string().length() + 1;
  fs::path to_path{""};

  for (auto i = fs::recursive_directory_iterator(link_parms.from_dir);
       i != fs::recursive_directory_iterator(); ++i) {
    to_path = link_parms.to_dir / i->path().string().substr(from_dir_len);
    if (i->is_directory(ec)) {
      fs::create_directory(to_path, link_parms.to_dir, ec);
      std::cout << std::string(indent + ((i.depth() + 1) * 2), ' ')
                << i->path().filename() << "\n";
    } else {
      fs::create_symlink(i->path(), to_path, ec);
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  std::optional<Link_parms> link_parms = parse_args(argc, argv);
  if (!link_parms) {
    std::cout << usage;
    return -1;
  }

  std::cout << "Linking:\n";
  std::cout << "  from dir: " << link_parms->from_dir << "\n";
  std::cout << "  to dir:   " << link_parms->to_dir << "\n";
  std::cout << "  directories linked:\n";
  std::cout << "    " << link_parms->from_dir.filename() << "\n";

  link_dir_trees(*link_parms);

  return 0;
}
