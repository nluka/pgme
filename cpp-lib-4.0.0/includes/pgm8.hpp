#ifndef CPPLIB_PGM8_HPP
#define CPPLIB_PGM8_HPP

#include <fstream>
#include <cinttypes>
#include <string>
#include <vector>
#include "arr2d.hpp"

// stands for `portable gray map 8-bit`
namespace pgm8 {

enum class Type {
  PLAIN = 2,
  RAW = 5,
};

/*
  Writes an uncompressed 8-bit PGM image.
  If writing a raw (pgm8::Type::RAW) file, make sure `file` is in binary
  (std::ios::binary) mode!
*/
void write(
  std::ofstream *file,
  uint16_t width,
  uint16_t height,
  uint8_t maxval,
  uint8_t const *pixels,
  pgm8::Type type = pgm8::Type::RAW
);

// Class for reading 8-bit PGM image files.
class Image {
private:
  uint_fast16_t m_width, m_height;
  uint8_t *m_pixels, m_maxval;

public:
  Image();
  Image(std::ifstream &file);
  ~Image();
  void load(std::ifstream &file);
  uint_fast16_t width() const;
  uint_fast16_t height() const;
  uint8_t *pixels() const;
  size_t pixel_count() const;
  uint8_t maxval() const;
  // TODO: implement move and copy ops
};

// Class for Run-length encoding.
class RLE {
public:
  struct Chunk {
    uint8_t const m_data;
    uint32_t const m_count;

    Chunk(uint8_t const data, uint32_t const count)
    : m_data{data}, m_count{count}
    {}

    bool operator!=(Chunk const &other) const {
      return m_data != other.m_data || m_count != other.m_count;
    }
  };

  RLE();
  RLE(uint8_t const *pixels, size_t pixelCount);
  RLE(uint8_t const *pixels, uint16_t width, uint16_t height);
  void encode(
    uint8_t const *pixels,
    size_t pixelCount,
    bool clearExistingChunks = true
  );
  static uint8_t *decode(std::vector<Chunk> const &);
  void write_chunks_to_file(std::ofstream &);
  void load_file_chunks(std::ifstream &);
  std::vector<Chunk> const &chunks() const;

private:
  std::vector<Chunk> m_chunks{};
};

} // namespace pgm

#endif // CPPLIB_PGM8_HPP
