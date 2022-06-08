#ifndef CPPLIB_TEST_HPP
#define CPPLIB_TEST_HPP

#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include "term.hpp"

namespace test {

//                  name
#define CASE(expr) (#expr), (expr)

void use_stdout(bool);
void set_ofstream(std::ofstream *);
void set_indentation(char const *);
void set_verbose_mode(bool);

class Suite {
  class Assertion {
  public:
    std::string const m_name;
    bool const m_expr;

    Assertion() : m_name{}, m_expr{false} {}
    Assertion(char const *const name, bool const expr)
      : m_name{name}, m_expr{expr} {}
  };

private:
  std::vector<Assertion> m_assertions{};

public:
  std::string const m_name;

  Suite() = delete;
  Suite(char const *const name) : m_name{name} {}

  void assert(char const *const name, bool const expr);
  void print_assertions() const;
  size_t passes() const;
  size_t fails() const;
};

void register_suite(Suite &&);
void evaluate_suites();

} // namespace test

#endif // CPPLIB_TEST_HPP
