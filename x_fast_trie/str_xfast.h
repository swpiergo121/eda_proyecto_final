#ifndef STR_XFAST_H_
#define STR_XFAST_H_

#include "xfast.h"
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

class StringXFastTrie {
public:
  explicit StringXFastTrie(size_t max_string_length)
      : trie_(static_cast<uint32_t>(max_string_length * 8)),
        max_len_(max_string_length) {
    if (max_string_length == 0 || max_string_length > 8) {
      throw std::runtime_error(
          "max_string_length must be between 1 and 8 (uint64_t limit)");
    }
  }

  void insert(const std::string &s) {
    uint64_t k = encode(s);
    if (trie_.find(k))
      return; // already present
    trie_.insert(k);
    originals_[k] = s;
  }

  bool find(const std::string &s) { return trie_.find(encode(s)); }

  std::optional<std::string> successor(const std::string &s) {
    uint64_t k = encode(s);
    uint64_t succ = trie_.strict_successor(k);
    if (succ == 0)
      return std::nullopt;
    return originals_.at(succ);
  }

  std::optional<std::string> predecessor(const std::string &s) {
    uint64_t k = encode(s);
    uint64_t pred = trie_.strict_predecessor(k);
    if (pred == 0)
      return std::nullopt;
    return originals_.at(pred);
  }

  std::optional<std::string> min() const {
    uint64_t k = trie_.min_key();
    if (k == 0)
      return std::nullopt;
    return originals_.at(k);
  }

  std::optional<std::string> max() const {
    uint64_t k = trie_.max_key();
    if (k == 0)
      return std::nullopt;
    return originals_.at(k);
  }

  int size() const { return trie_.leaf_count(); }

private:
  x_fast_trie trie_;
  size_t max_len_;
  std::unordered_map<uint64_t, std::string> originals_;

  uint64_t encode(const std::string &s) const {
    if (s.empty()) {
      throw std::runtime_error("Empty strings are not allowed");
    }
    if (s.size() > max_len_) {
      throw std::runtime_error("String exceeds maximum length");
    }
    if (s.find('\0') != std::string::npos) {
      throw std::runtime_error("Null bytes are not allowed in strings");
    }

    uint64_t key = 0;
    for (unsigned char c : s) {
      key = (key << 8) | c;
    }
    key <<= static_cast<uint64_t>(8 * (max_len_ - s.size()));
    return key;
  }
};

#endif // STR_XFAST_H_
