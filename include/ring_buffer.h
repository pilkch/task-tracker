#pragma once

#include <array>
#include <span>

namespace util {

// ** ring_buffer
//
// A ring buffer, but it has a variable size up to the maximum N
//
template <class T, size_t N>
class ring_buffer
{
public:
  ring_buffer();

  void push_back(const T& item);
  void push_back(std::span<T> items);

  constexpr size_t size() const;
  constexpr bool empty() const;

  T& operator[](size_t i);
  constexpr const T& operator[](size_t i) const;

private:
  std::array<T, N> elements;
  size_t first;
  size_t n;
};

template <class T, size_t N>
ring_buffer<T, N>::ring_buffer() :
  first(0),
  n(0)
{
}

template <class T, size_t N>
void ring_buffer<T, N>::push_back(const T& item)
{
  if (n < N) {
    // We aren't at the limit yet so fill in next item and expand our range
    elements[(first + n) % N] = item;
    n++;
  } else {
    // We have already filled the buffer, overwrite the oldest entry and shuffle our range along
    elements[first] = item;
    first++;
    if (first >= N) first = 0;
  }
}

template <class T, size_t N>
void ring_buffer<T, N>::push_back(std::span<T> items)
{
  for (auto&& item : items) {
    push_back(item);
  }
}

template <class T, size_t N>
constexpr size_t ring_buffer<T, N>::size() const
{
  return n;
}

template <class T, size_t N>
constexpr bool ring_buffer<T, N>::empty() const
{
  return (n == 0);
}

template <class T, size_t N>
constexpr const T& ring_buffer<T, N>::operator[](size_t i) const
{
  return elements[(first + i) % N];
}

template <class T, size_t N>
T& ring_buffer<T, N>::operator[](size_t i)
{
  return elements[(first + i) % N];
}

}
