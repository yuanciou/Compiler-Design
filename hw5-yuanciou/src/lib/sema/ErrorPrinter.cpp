#include "sema/ErrorPrinter.hpp"

#include <cstdint>
#include <cstdio>
#include <string>

#include "AST/ast.hpp"

extern FILE *yyin;
extern long line_positions[];

ErrorPrinter::ErrorPrinter(std::FILE *p_file) : m_file{p_file} {}

void ErrorPrinter::print(const Error &p_error) const {
  std::fprintf(m_file, "<Error> Found in line %d, column %d: %s\n",
               p_error.getLocation().line, p_error.getLocation().col,
               p_error.getMessage().c_str());

  constexpr uint32_t kIndentionWidth = 4;
  if (std::fseek(yyin, line_positions[p_error.getLocation().line], SEEK_SET) ==
      0) {
    char buffer[512];
    std::fgets(buffer, sizeof(buffer), yyin);
    std::fprintf(m_file, "%*s%s", kIndentionWidth, "", buffer);
    std::fprintf(m_file, "%*s\n", kIndentionWidth + p_error.getLocation().col,
                 "^");
  } else {
    std::fprintf(m_file, "Fail to reposition the yyin file stream.\n");
  }
}
