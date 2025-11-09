#ifndef RDS_ITERATOR_HPP
#define RDS_ITERATOR_HPP

#include "rds/common.hpp"

#include <iterator>
#include <memory>
#include <vector>

namespace rds {

/**
 * A wrapper around std::vector<std::unique_pt<T>>::iterator to iterate a
 * std::vector<std::unique_ptr<T>> like a std::vector<T>.
 *
 * @tparam T the element type
 */
template <typename T> class Iterator : std::random_access_iterator_tag {
  using Iter = typename std::vector<std::unique_ptr<T>>::iterator;
  Iter iter_{};

  friend struct Device;
  friend struct Group;
  friend struct Register;
  friend struct Field;
  Iterator(Iter iter) : iter_(iter) {}

public:
  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;
  using difference_type = typename Iter::difference_type;

  Iterator() = default;
  Iterator(const Iterator &) = default;
  Iterator(Iterator &&) = default;
  Iterator &operator=(const Iterator &) = default;
  Iterator &operator=(Iterator &&) = default;

  Iterator &operator++() {
    ++iter_;
    return *this;
  }

  Iterator operator++(int) {
    auto i = iter_;
    ++iter_;
    return i;
  }

  Iterator &operator--() {
    --iter_;
    return *this;
  }

  Iterator operator--(int) {
    auto i = iter_;
    --iter_;
    return i;
  }

  Iterator &operator+=(difference_type n) {
    iter_ += n;
    return *this;
  }

  friend Iterator operator+(const Iterator &it, difference_type n) {
    return {it.iter_ + n};
  }

  friend Iterator operator+(difference_type n, const Iterator &it) {
    return {it.iter_ + n};
  }

  Iterator &operator-=(difference_type n) {
    iter_ -= n;
    return *this;
  }

  friend Iterator operator-(const Iterator &it, difference_type n) {
    return {it.iter_ - n};
  }

  friend Iterator operator-(difference_type n, const Iterator &it) {
    return {it.iter_ - n};
  }

  friend difference_type operator-(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ - it2.iter_;
  }

  Iterator operator[](difference_type n) const { return iter_[n]; }

  reference operator*() { return *(iter_->get()); }
  const_reference operator*() const { return *(iter_->get()); }
  pointer operator->() { return iter_->get(); }
  const_pointer operator->() const { return iter_->get(); }

  friend bool operator==(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ == it2.iter_;
  }

  friend bool operator!=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ != it2.iter_;
  }

  friend bool operator<(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ < it2.iter_;
  }

  friend bool operator<=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ <= it2.iter_;
  }

  friend bool operator>(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ > it2.iter_;
  }

  friend bool operator>=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ >= it2.iter_;
  }
};

/**
 * A wrapper around std::vector<std::unique_pt<T>>::const_iterator to iterate a
 * std::vector<std::unique_ptr<T>> like a std::vector<T>.
 *
 * @tparam T the element type
 */
template <typename T>
class Iterator<const T> : std::random_access_iterator_tag {
  using Iter = typename std::vector<std::unique_ptr<T>>::const_iterator;
  Iter iter_{};

  friend struct Device;
  friend struct Group;
  friend struct Register;
  friend struct Field;
  Iterator(Iter iter) : iter_(iter) {}

public:
  using value_type = const T;
  using reference = const T &;
  using pointer = const T *;
  using difference_type = typename Iter::difference_type;

  Iterator() = default;
  Iterator(const Iterator &) = default;
  Iterator(Iterator &&) = default;
  Iterator &operator=(const Iterator &) = default;
  Iterator &operator=(Iterator &&) = default;

  Iterator &operator++() {
    ++iter_;
    return *this;
  }

  Iterator operator++(int) {
    auto i = iter_;
    ++iter_;
    return i;
  }

  Iterator &operator--() {
    --iter_;
    return *this;
  }

  Iterator operator--(int) {
    auto i = iter_;
    --iter_;
    return i;
  }

  Iterator &operator+=(difference_type n) {
    iter_ += n;
    return *this;
  }

  friend Iterator operator+(const Iterator &it, difference_type n) {
    return {it.iter_ + n};
  }

  friend Iterator operator+(difference_type n, const Iterator &it) {
    return {it.iter_ + n};
  }

  Iterator &operator-=(difference_type n) {
    iter_ -= n;
    return *this;
  }

  friend Iterator operator-(const Iterator &it, difference_type n) {
    return {it.iter_ - n};
  }

  friend Iterator operator-(difference_type n, const Iterator &it) {
    return {it.iter_ - n};
  }

  friend difference_type operator-(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ - it2.iter_;
  }

  Iterator operator[](difference_type n) const { return iter_[n]; }

  reference operator*() const { return *(iter_->get()); }
  pointer operator->() const { return iter_->get(); }

  friend bool operator==(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ == it2.iter_;
  }

  friend bool operator!=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ != it2.iter_;
  }

  friend bool operator<(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ < it2.iter_;
  }

  friend bool operator<=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ <= it2.iter_;
  }

  friend bool operator>(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ > it2.iter_;
  }

  friend bool operator>=(const Iterator &it1, const Iterator &it2) {
    return it1.iter_ >= it2.iter_;
  }
};

} // namespace rds
#endif
