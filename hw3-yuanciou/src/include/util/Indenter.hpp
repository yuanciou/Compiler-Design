#ifndef UTIL_INDENTER_HPP
#define UTIL_INDENTER_HPP

#include <cstddef>
#include <string>

class Indenter {
 public:
  /// @return `size_per_level` * `level` number of `symbol`s.
  std::string indent() const;

  void increaseLevel();

  /// @note The indention level saturates at 0.
  void decreaseLevel();

  /// @param p_symbol The p_symbol used to indent with.
  /// @param p_size_per_level The indention size. For example, if the size is
  /// `2`, each indention level adds 2 `p_symbol`s.
  /// @param p_max_level If equals to `0`, there is no limit.
  Indenter(char p_symbol, std::size_t p_size_per_level,
           std::size_t p_max_level = 0)
      : m_symbol{p_symbol},
        m_size_per_level{p_size_per_level},
        m_max_level{p_max_level} {}

 private:
  char m_symbol;
  std::size_t m_size_per_level;
  std::size_t m_max_level;
  /// @brief The current indention level.
  std::size_t m_level{0};

  bool hasNoLevelLimit() const;
};

#endif  // UTIL_INDENTER_HPP
