#include <iostream>
#include "../cpp-lib-4.0.0/includes/cstr.hpp"
#include "../cpp-lib-4.0.0/includes/term.hpp"
#include "../cpp-lib-4.0.0/includes/pgm8.hpp"

enum class Action {
  // UNKNOWN = 0,
  ENCODE,
  DECODE,
};

template<typename FstreamType>
void assert_file(FstreamType const *file, char const *fpathname) {
  if (!file->is_open()) {
    term::printf_colored(
      term::ColorText::RED,
      "failed to open file `%s`",
      fpathname
    );
    exit(-1);
  }
}

int main(int const argc, char const *const *const argv) {
  if (argc < 3) {
    term::printf_colored(term::ColorText::YELLOW, "usage: <action> <file>\n");
    std::exit(1);
  }

  Action const action = ([argv](){
    if (cstr::len(argv[1]) == 1) {
      switch (argv[1][0]) {
        case 'e':
        case 'E':
        case 'c':
        case 'C':
          return Action::ENCODE;
        case 'd':
        case 'D':
          return Action::DECODE;
        default:
          std::cerr << "fatal: invalid <action>";
          std::exit(2);
          break;
      }
    } else if (
      cstr::cmp(argv[1], "encode") == 0 ||
      cstr::cmp(argv[1], "enc") == 0 ||
      cstr::cmp(argv[1], "compress") == 0 ||
      cstr::cmp(argv[1], "comp") == 0
    ) {
      return Action::ENCODE;
    } else if (
      cstr::cmp(argv[1], "decode") == 0 ||
      cstr::cmp(argv[1], "dec") == 0
    ) {
      return Action::DECODE;
    } else {
      std::cerr << "fatal: invalid <action>";
      std::exit(2);
    }
  })();

  try {
    char const *const inFilePathname = argv[2];
    switch (action) {
      case Action::ENCODE: {
        std::ifstream inFile(inFilePathname);
        assert_file(&inFile, inFilePathname);
        pgm8::Image img(inFile);
        inFile.close();

        pgm8::RLE encoding(img.pixels(), img.pixel_count());

        std::string const outFilePathname(std::string(inFilePathname) + "c");
        std::ofstream outFile(outFilePathname);
        assert_file(&outFile, outFilePathname.c_str());

        // header
        outFile << "PGME\n"
          << std::to_string(img.width()) << ' ' << std::to_string(img.height()) << '\n'
          << std::to_string(img.maxval()) << '\n';
        // pixels
        encoding.write_chunks_to_file(outFile);
        outFile.close();

        break;
      }
      case Action::DECODE:

        break;
    }
  } catch (char const *const err) {
    std::cerr << "fatal: " << err << '\n';
    std::exit(3);
  }

  return 0;
}
