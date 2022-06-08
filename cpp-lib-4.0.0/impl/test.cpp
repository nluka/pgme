#include <iostream>
#include "../includes/test.hpp"

using test::Suite;

static bool s_useStdout = true;
void test::use_stdout(bool const b) {
  s_useStdout = b;
}

static std::ofstream *s_ofstream = nullptr;
void test::set_ofstream(std::ofstream *const ofs) {
  s_ofstream = ofs;
}

static char const *s_indent = "\t";
void test::set_indentation(char const *const i) {
  s_indent = i;
}

static bool s_verboseMode = false;
void test::set_verbose_mode(bool const b) {
  s_verboseMode = b;
}

void Suite::assert(char const *const name, bool const expr) {
  m_assertions.emplace_back(name, expr);
}

void Suite::print_assertions() const {
  auto const print = [](std::ostream *const os, Assertion const &a){
    bool const passed = a.m_expr;
    if (!passed || (passed && s_verboseMode)) {
      *os << s_indent << (passed ? "pass" : "fail") << ": " << a.m_name << '\n';
    }
  };

  for (auto const &a : m_assertions) {
    if (s_useStdout) {
      print(&std::cout, a);
    }
    if (s_ofstream != nullptr) {
      print(s_ofstream, a);
    }
  }
}

size_t Suite::passes() const {
  return std::count_if(
    m_assertions.begin(), m_assertions.end(),
    [](Assertion const &a){
      return a.m_expr == true;
    }
  );
}

size_t Suite::fails() const {
  return std::count_if(
    m_assertions.begin(), m_assertions.end(),
    [](Assertion const &a){
      return a.m_expr == false;
    }
  );
}

static std::vector<Suite> s_suites{};
void test::register_suite(Suite &&s) {
  s_suites.push_back(s);
}

void test::evaluate_suites() {
  for (auto const &s : s_suites) {
    auto const printHeader = [&s](std::ostream *const os){
      size_t const passes = s.passes(), cases = passes + s.fails();
      *os << s.m_name << " (" << passes << '/' << cases << ")\n";
    };

    if (s_useStdout) {
      printHeader(&std::cout);
    }
    if (s_ofstream != nullptr) {
      printHeader(s_ofstream);
    }

    s.print_assertions();
  }

  s_suites.clear();
}
