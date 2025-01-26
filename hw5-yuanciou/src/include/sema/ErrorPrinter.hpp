#ifndef SEMA_ERROR_PRINTER_HPP
#define SEMA_ERROR_PRINTER_HPP

#include "sema/Error.hpp"
#include <cstdio>

class ErrorPrinter {
public:
  /// @brief Prints the location of the error, the description of the error, and
  /// the source line that causes the error.
  void print(const Error &) const;

  /// @param p_file The file to print the error to. The caller is responsible
  /// for ensuring the `p_file` is valid throughout the print and closing the
  /// `p_file` after use.
  explicit ErrorPrinter(std::FILE *p_file);

private:
  std::FILE *m_file;
};

#endif // SEMA_ERROR_PRINTER_HPP
