#ifndef CPPLIB_LOGGER_HPP
#define CPPLIB_LOGGER_HPP

#include <string>

namespace logger {

void set_out_pathname(char const *);
void set_out_pathname(std::string const &);
void set_max_msg_len(size_t);
void set_delim(char const *);
void set_autoflush(bool);

enum class EventType {
  INF,
  WRN,
  ERR,
  FTL
};

void write(EventType, char const *fmt, ...);
void flush();

} // namespace log

#endif // CPPLIB_LOGGER_HPP
