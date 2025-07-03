#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace view
{

template <typename T> class vector_view
{
public:
  using value_type = std::remove_cv_t<T>;
  using pointer = T *;
  using reference = T &;
  using const_pointer = const T *;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = pointer;
  using const_iterator = const_pointer;

  // Constructors
  constexpr vector_view () noexcept : data_ (nullptr), size_ (0) {}

  constexpr vector_view (pointer data, size_type size) noexcept
      : data_ (data), size_ (size)
  {
  }

  constexpr vector_view (iterator begin, iterator end) noexcept
      : data_ (begin), size_ (std::distance (begin, end))
  {
  }

  template <std::size_t N>
  constexpr vector_view (T (&arr)[N]) noexcept : data_ (arr), size_ (N)
  {
  }

  template <
      typename Container,
      typename = std::enable_if_t<
          std::is_convertible_v<
              decltype (std::data (std::declval<Container &> ())), pointer>
          && std::is_convertible_v<
              decltype (std::size (std::declval<Container &> ())), size_type>>>
  constexpr vector_view (Container &c)
      : data_ (std::data (c)), size_ (std::size (c))
  {
  }

  // Iterators
  constexpr iterator
  begin () const noexcept
  {
    return data_;
  }
  constexpr iterator
  end () const noexcept
  {
    return data_ + size_;
  }
  constexpr const_iterator
  cbegin () const noexcept
  {
    return data_;
  }
  constexpr const_iterator
  cend () const noexcept
  {
    return data_ + size_;
  }

  // Capacity
  constexpr size_type
  size () const noexcept
  {
    return size_;
  }
  constexpr bool
  empty () const noexcept
  {
    return size_ == 0;
  }

  // Element access
  constexpr reference
  operator[] (size_type idx) const
  {
    return data_[idx];
  }
  constexpr reference
  at (size_type idx) const
  {
    if (idx >= size_)
      throw std::out_of_range ("vector_view::at");
    return data_[idx];
  }
  constexpr reference
  front () const
  {
    return data_[0];
  }
  constexpr reference
  back () const
  {
    return data_[size_ - 1];
  }
  constexpr pointer
  data () const noexcept
  {
    return data_;
  }

  // Subview
  constexpr vector_view
  subview (size_type offset, size_type count) const
  {
    if (offset > size_ || offset + count > size_)
      throw std::out_of_range ("vector_view::subview");
    return vector_view (data_ + offset, count);
  }

private:
  pointer data_;
  size_type size_;
};

} // namespace view
