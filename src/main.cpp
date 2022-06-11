#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <filesystem>
#include "../cpp-lib-4.0.0/includes/cstr.hpp"
#include "../cpp-lib-4.0.0/includes/term.hpp"
#include "../cpp-lib-4.0.0/includes/pgm8.hpp"

namespace fs = std::filesystem;

enum class Action {
  ENCODE,
  DECODE,
};

template<typename FstreamType>
void assert_file(FstreamType const *file, char const *fpathname) {
  if (!file->is_open()) {
    std::stringstream err{};
    err << "failed to open file `" << fpathname << '`';
    throw err.str();
  }
}

int main(int const argc, char const *const *const argv) {
  if (argc < 4) {
    term::printf_colored(
      term::ColorText::YELLOW,
      "   usage: <action> <in_file> <out_file>\n"
      " actions: encode|enc|e|E|compress|comp|c|C\n"
      "          decode|decompress|dec|d|D\n"
    );
    std::exit(1);
  }

  try {
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
            throw std::string("invalid <action>");
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
        cstr::cmp(argv[1], "decompress") == 0 ||
        cstr::cmp(argv[1], "dec") == 0
      ) {
        return Action::DECODE;
      } else {
        throw std::string("invalid <action>");
      }
    })();

    fs::path const
      inFilePathname = argv[2],
      outFilePathname = argv[3];

    // verify input and output files are valid before we do any processing
    std::ifstream inFile(inFilePathname);
    assert_file(&inFile, inFilePathname.string().c_str());
    std::ofstream outFile(outFilePathname);
    assert_file(&outFile, outFilePathname.string().c_str());
    outFile.close(); // close for now because we may not need it for a while

    switch (action) {
      case Action::ENCODE: {
        pgm8::Image img(inFile);
        inFile.close();

        pgm8::RLE encoding(img.pixels(), img.pixel_count());

        outFile.open(outFilePathname);
        assert_file(&outFile, outFilePathname.string().c_str());
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
        std::string magicNum{};
        std::getline(inFile, magicNum);
        if (magicNum != "PGME") {
          throw std::string("invalid magic number");
        }

        int width = 0, height = 0, maxval = 0;
        inFile >> width >> height >> maxval;
        // read newline between maxval and pixels
        {
          char newline{};
          inFile.read(&newline, 1);
        }
        pgm8::RLE encoding{};
        encoding.load_file_chunks(inFile);
        inFile.close();

        std::unique_ptr<uint8_t const> pixels(
          pgm8::RLE::decode(encoding.chunks())
        );
        outFile.open(outFilePathname);
        pgm8::write(
          &outFile,
          static_cast<uint16_t>(width),
          static_cast<uint16_t>(height),
          static_cast<uint8_t>(maxval),
          pixels.get(),
          pgm8::Type::PLAIN
        );
        outFile.close();

        break;
    }
  } catch (std::string const &err) {
    term::printf_colored(term::ColorText::RED, "fatal: %s", err);
    std::exit(1);
  }

  return 0;
}
