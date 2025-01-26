#include "util/Indenter.hpp"

#include <string>

std::string Indenter::indent() const {
  return std::string(m_size_per_level * m_level, m_symbol);
}

void Indenter::increaseLevel() {
  if (hasNoLevelLimit() || m_level < m_max_level) {
    ++m_level;
  }
}

void Indenter::decreaseLevel() {
  if (m_level) {
    --m_level;
  }
}

bool Indenter::hasNoLevelLimit() const { return 0 == m_max_level; }
