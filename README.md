# lndir -- link Directory Tree

Create a shadow directory of symbolic links to another directory tree.

## Origin Story

Inspired by the X Window System utility of the same name. As such,
if one wanted to use this utility, on linux, they had to install
an X Window System package; for example, the
[`xutils-dev`](https://packages.ubuntu.com/artful/amd64/xutils-dev/filelist)
Ubuntu package.

Primilarly driven by the desire to learn the
[`std::filesystem`](http://en.cppreference.com/w/cpp/filesystem)
library, of [C++17](https://en.wikipedia.org/wiki/C%2B%2B17), I
was also motivated by the dream to one day make this utility
more accessible to people not using X Windows, or not interested
in installing such a package as `xutils-dev` in order to get
this single utility.

## Use Cases

### Multiple Versions of Compilers

In this use case, there is a need to make available multiple versions
of a C++ compiler, for the purpose of ensuring that a project's
source code compiles against the most recent versions of the compiler.

For example, assume that the `gcc` compiler is used, and that two
versions of the compiler will need to be made available: `7.3.1` and
`8.0.1`. As such, the following `bin` directory can be set up using `lndir`:

```
bin
├── compilers
├── c++ -> compilers/gcc/vers/8.0.1/bin/c++
├── c++-v7 -> compilers/gcc/vers/7.3.1/bin/c++
├── cpp -> compilers/gcc/vers/8.0.1/bin/cpp
├── cpp-v7 -> compilers/gcc/vers/7.3.1/bin/cpp
├── g++ -> compilers/gcc/vers/8.0.1/bin/g++
├── g++-v7 -> compilers/gcc/vers/7.3.1/bin/g++
├── gcc -> compilers/gcc/vers/8.0.1/bin/gcc
├── gcc-v7 -> compilers/gcc/vers/7.3.1/bin/gcc
├── gcc-ar -> compilers/gcc/vers/8.0.1/bin/gcc-ar
├── gcc-ar-v7 -> compilers/gcc/vers/7.3.1/bin/gcc-ar
├── gcc-nm -> compilers/gcc/vers/8.0.1/bin/gcc-nm
├── gcc-nm-v7 -> compilers/gcc/vers/7.3.1/bin/gcc-nm
├── gcc-ranlib -> compilers/gcc/vers/8.0.1/bin/gcc-ranlib
├── gcc-ranlib-v7 -> compilers/gcc/vers/7.3.1/bin/gcc-ranlib
├── gcov -> compilers/gcc/vers/8.0.1/bin/gcov
├── gcov-v7 -> compilers/gcc/vers/7.3.1/bin/gcov
├── gcov-dump -> compilers/gcc/vers/8.0.1/bin/gcov-dump
├── gcov-dump-v7 -> compilers/gcc/vers/7.3.1/bin/gcov-dump
├── gcov-tool -> compilers/gcc/vers/8.0.1/bin/gcov-tool
├── gcov-tool-v7 -> compilers/gcc/vers/7.3.1/bin/gcov-tool
├── x86_64-pc-linux-gnu-c++ -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-c++
├── x86_64-pc-linux-gnu-c++-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-c++
├── x86_64-pc-linux-gnu-gcc -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-gcc
├── x86_64-pc-linux-gnu-gcc-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-gcc
├── x86_64-pc-linux-gnu-gcc-7.3-v7.1 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-gcc-7.3.1
├── x86_64-pc-linux-gnu-gcc-8.0.1 -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-gcc-8.0.1
├── x86_64-pc-linux-gnu-gcc-ar -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-gcc-ar
├── x86_64-pc-linux-gnu-gcc-ar-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-gcc-ar
├── x86_64-pc-linux-gnu-gcc-nm -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-gcc-nm
├── x86_64-pc-linux-gnu-gcc-nm-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-gcc-nm
├── x86_64-pc-linux-gnu-gcc-ranlib -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-gcc-ranlib
├── x86_64-pc-linux-gnu-gcc-ranlib-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-gcc-ranlib
├── x86_64-pc-linux-gnu-g++ -> compilers/gcc/vers/8.0.1/bin/x86_64-pc-linux-gnu-g++
└── x86_64-pc-linux-gnu-g++-v7 -> compilers/gcc/vers/7.3.1/bin/x86_64-pc-linux-gnu-g++
```

In the above directory structure, where the top level directory is a `bin`
directory, there is a sub-directory called `compilers` which contains the
binary files of each version of `gcc` that we require:
`./compilers/gcc/vers/7.3.1` and `./compilers/gcc/vers/8.0.1`.

Let's assume that we want the most recent version of `gcc`, that is `8.0.1`,
to be the default compiler. As such, we would like the names of the `8.0.1`
executables to be the unadorned defaults:  `gcc`, `g++`, etc. Therefore,
in order for both versions of the executables to coexist in the same directory,
we will add a suffix for the `7.3.1` binaries: `-v7`. For example, we require
the following filenames for the `c++` binary:

Version | Filename
--------|---------
8.0.1   | `c++`
7.3.1   | `c++-v7`

To achieve the co-existence of these two versions of the `gcc` binaries,
using this filename convention to distinguish the different versions of
the binaries, the following could be how you use `lndir` to set this
up, given that the sub-directories of these two versions of `gcc` are
already populated:  `compilers/gcc/vers/7.3.1` and `compilers/gcc/vers/8.0.1`:

```bash
cd $GCC_BIN_DIR
lndir compilers/gcc/vers/8.0.1/bin
lndir --suffix -v7 compilers/gcc/vers/7.3.1/bin
```

### Source/Binary Files of Different Machine Architectures

Consider the case where a project contains source code, or binaries, for
different machine architectures. Further, assume that you need to make
available the appropriate set of files on various computers, of differing
architectures. That is, if the full multi-architecture file set is mounted
onto each computer from a remote server, then you need to ensure that
computer A, of architecture `amd64`, accesses only the `amd64` set of files
from the remote mount; whereas computer B, of architecture `arm64`, sees
only the `arm64` files; and so on. The `lndir` tool can be used to achieve
this linking of architecture-specific files to computers.

## Usage

### Name

_lndir_ - create shadow directory of symlinks to another directory tree

### Syntax

```
lndir [options] from-dir [to-dir]
```

### Description

The `lndir` tool makes a shadow copy **to-dir** of a directory tree
**from-dir**, except that the shadow is not populated with real files
but instead with symbolic links pointing at the real files in the
**from-dir** directory tree.

When **to-dir** is not specified, it defaults to the current directory,
from which `lndir` is run.

### Options

`--suffix <suffix>`

* Append the text `<suffix>` to each link in the **to-dir**
* For example, given `--suffix -v7`, the file `from-dir/foo` will be linked as `to-dir/foo-v7`

`--help`

* Display the usage help

## Building

### Prerequisites

* [C++17](https://en.wikipedia.org/wiki/C%2B%2B17) compiler with a [`std::filesystem`](http://en.cppreference.com/w/cpp/filesystem) implementation
    * note, the `cmake` build configuration includes a test for a compatible compiler
* [`cmake`](https:://cmake.org), minimum version of 3.8.2.

### Build Steps

In the following example of building the `lndir` tool, the `LNDIR_SOURCE_DIR`
environment variable is set to the directory in which the source code of
`lndir` is downloaded and compiled. For example, `LNDIR_SOURCE_DIR=/usr/local/src/lndir`.

```bash
git clone https://github.com/enigmata/lndir.git $LNDIR_SOURCE_DIR
cd $LNDIR_SOURCE_DIR
mkdir build
cd build
cmake ..
make
```

### Compatible Platforms

Successful builds have been performed on these operating systems:

* Linux - Ubuntu 17.10 (artful)
* Mac OS 10.13

However, it should be possible to build on other versions of Mac OS and
other versions and distributions of Linux, as the key requirement is
a version of a C++17 compiler supporting `std::filesystem`.

## Installation

## Testing

