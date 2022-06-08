#include <string>
#include <sstream>
#include <numeric>
#include "../includes/cstr.hpp"
#include "../includes/pgm8.hpp"

void pgm8::write(
  std::ofstream *const file,
  uint16_t const width,
  uint16_t const height,
  uint8_t const maxval,
  uint8_t const *const pixels,
  Type const type
) {
  using pgm8::Type;

  if (maxval < 1) {
    throw "pgm8::write failed: maxval must be > 0";
  }

  // header
  {
    char magicNum[3] {
      'P',
      cstr::int_to_ascii_digit(static_cast<int>(type)),
      '\0'
    };

    *file << magicNum << '\n'
      << std::to_string(width) << ' ' << std::to_string(height) << '\n'
      << std::to_string(maxval) << '\n';
  }

  // pixels
  switch (type) {
    case Type::PLAIN: {
      for (uint16_t r = 0; r < height; ++r) {
        for (uint16_t c = 0; c < width; ++c) {
          *file
            << static_cast<int>(pixels[arr2d::get_1d_idx(width, c, r)])
            << ' ';
        }
        *file << '\n';
      }
      break;
    }
    case Type::RAW: {
      size_t const pixelCount =
        static_cast<size_t>(width) * static_cast<size_t>(height);
      file->write(reinterpret_cast<char const *>(pixels), pixelCount);
      break;
    }
    default: {
      throw "pgm8::write failed: `type` is not a valid pgm8::Type";
    }
  }
}

using pgm8::RLE;

RLE::RLE() {}

RLE::RLE(uint8_t const *pixels, size_t const pixelCount) {
  encode(pixels, pixelCount);
}

RLE::RLE(uint8_t const *pixels, uint16_t const width, uint16_t const height) {
  size_t const pixelCount =
    static_cast<size_t>(width) * static_cast<size_t>(height);
  encode(pixels, pixelCount);
}

void RLE::encode(
  uint8_t const *const pixels,
  size_t const pixelCount,
  bool const clearExistingChunks
) {
  if (clearExistingChunks) {
    m_chunks.clear();
  }

  size_t pos = 0;

  while (pos < pixelCount) {
    uint8_t const data = pixels[pos];
    uint32_t count = 0;
    do {
      ++count;
      ++pos;
    } while (pos < pixelCount && pixels[pos] == data);
    m_chunks.emplace_back(data, count);
  }
}

uint8_t *RLE::decode(std::vector<Chunk> const &chunks) {
  size_t const pixelCount = ([&chunks](){
    size_t cnt = 0;
    for (auto const &chunk : chunks) {
      cnt += chunk.m_count;
    }
    return cnt;
  })();

  if (pixelCount == 0) {
    return nullptr;
  }

  uint8_t *const pixels = new uint8_t[pixelCount];
  size_t pos = 0;
  for (auto const &chunk : chunks) {
    std::fill_n(pixels + pos, chunk.m_count, chunk.m_data);
    pos += chunk.m_count;
  }
  return pixels;
}

void RLE::write_chunks_to_file(std::ofstream &file) {
  if (!file.is_open()) {
    throw "RLE::write_chunks_to_file failed: file not open";
  }
  if (!file.good()) {
    throw "RLE::write_chunks_to_file failed: bad file";
  }

  // write number of chunks for easier reading
  {
    size_t const chunkCount = m_chunks.size();
    file.write(
      reinterpret_cast<char const *>(&chunkCount),
      sizeof(size_t)
    );
  }

  // write chunk data
  for (auto const &chunk : m_chunks) {
    file.write(
      reinterpret_cast<char const *>(&chunk.m_data),
      sizeof(chunk.m_data)
    );
    file.write(
      reinterpret_cast<char const *>(&chunk.m_count),
      sizeof(chunk.m_count)
    );
  }
}

void RLE::load_file_chunks(std::ifstream &file) {
  if (!file.is_open()) {
    throw "RLE::load_file_chunks failed: file not open";
  }
  if (!file.good()) {
    throw "RLE::load_file_chunks failed: bad file";
  }

  m_chunks.clear();

  // read number of chunks in file
  size_t chunkCount = 0;
  file.read(reinterpret_cast<char *>(&chunkCount), sizeof(size_t));
  m_chunks.reserve(chunkCount);

  // read chunk data
  for (uint32_t i = 0; i < chunkCount; ++i) {
    uint8_t data;
    file.read(reinterpret_cast<char *>(&data), sizeof(data));
    uint32_t count;
    file.read(reinterpret_cast<char *>(&count), sizeof(count));
    m_chunks.emplace_back(data, count);
  }
}

std::vector<RLE::Chunk> const &RLE::chunks() const {
  return m_chunks;
}

using pgm8::Image;

Image::Image()
: m_width{0}, m_height{0}, m_pixels{nullptr}, m_maxval{0}
{}

Image::Image(std::ifstream &file) : Image::Image() {
  load(file);
}

Image::~Image() {
  delete[] m_pixels;
}

void Image::load(std::ifstream &file) {
  using pgm8::Type;

  // clear prev image
  {
    if (m_pixels != nullptr) {
      delete[] m_pixels;
      m_pixels = nullptr;
    }
    m_width = 0;
    m_height = 0;
    m_maxval = 0;
  }

  if (!file.is_open()) {
    throw "pgm8::Image::load failed: file closed";
  }
  if (!file.good()) {
    throw "pgm8::Image::load failed: bad file";
  }

  Type const type = ([&file](){
    char magicNum[3] {};
    file.getline(magicNum, sizeof(magicNum));
    if (magicNum[0] != 'P' || (magicNum[1] != '2' && magicNum[1] != '5')) {
      throw "pgm8::Image::load failed: invalid magic number";
    }
    return static_cast<Type>(cstr::ascii_digit_to_int(magicNum[1]));
  })();

  file >> m_width >> m_height;
  {
    int maxval;
    file >> maxval;
    m_maxval = static_cast<uint8_t>(maxval);
  }

  // read newline between maxval and pixel data
  {
    char newline;
    file.read(&newline, 1);
  }

  const size_t pixelCount =
    static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
  try {
    m_pixels = new uint8_t[pixelCount];
  } catch (...) {
    throw "pgm8::Image::load failed: not enough memory";
  }

  if (type == Type::PLAIN) {
    char pixel[4] {};
    for (size_t i = 0; i < pixelCount; ++i) {
      file >> pixel;
      m_pixels[i] = static_cast<uint8_t>(std::stoul(pixel));
    }
  } else {
    file.read(reinterpret_cast<char *>(m_pixels), pixelCount);
  }
}

uint_fast16_t Image::width() const {
  return m_width;
}
uint_fast16_t Image::height() const {
  return m_height;
}
uint8_t *Image::pixels() const {
  return m_pixels;
}
size_t Image::pixel_count() const {
  return static_cast<size_t>(m_width) * static_cast<size_t>(m_height);
}
uint8_t Image::maxval() const {
  return m_maxval;
}
