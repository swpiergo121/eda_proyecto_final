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
      : trie_(static_cast<trie_key>(max_string_length * 8)),
        max_len_(max_string_length) {
    if (max_string_length == 0 || max_string_length > 16) {
      throw std::runtime_error(
          "max_string_length must be between 1 and 16 (trie_key limit)");
    }
  }

  void insert(const std::string &s) {
    trie_key k = encode(s);
    if (trie_.find(k))
      return; // already present
    trie_.insert(k);
    originals_[k] = s;
  }

  bool find(const std::string &s) { return trie_.find(encode(s)); }

  std::vector<std::string> get_closest_strings(const std::string &s, int n) {
    trie_key k = encode(s);
    std::vector<trie_key> closest_keys = trie_.get_closest(k, n);

    std::vector<std::string> result;
    for (trie_key key : closest_keys) {
      // Use the originals map to turn the key back into a string
      result.push_back(originals_.at(key));
    }
    return result;
  }



  std::optional<std::string> min() const {
    trie_key k = trie_.min_key();
    if (k == 0)
      return std::nullopt;
    return originals_.at(k);
  }

  std::optional<std::string> max() const {
    trie_key k = trie_.max_key();
    if (k == 0)
      return std::nullopt;
    return originals_.at(k);
  }

  int size() const { return trie_.leaf_count(); }

private:
  x_fast_trie trie_;
  size_t max_len_;
  std::unordered_map<trie_key, std::string> originals_;

  trie_key encode(const std::string &s) const {
    if (s.empty()) {
      throw std::runtime_error("Empty strings are not allowed");
    }
    if (s.size() > max_len_) {
      throw std::runtime_error("String exceeds maximum length");
    }
    if (s.find('\0') != std::string::npos) {
      throw std::runtime_error("Null bytes are not allowed in strings");
    }

    trie_key key = 0;
    for (unsigned char c : s) {
      key = (key << 8) | c;
    }
    key <<= static_cast<trie_key>(8 * (max_len_ - s.size()));
    return key;
  }
};

#endif // STR_XFAST_H_
