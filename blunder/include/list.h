#pragma once

#include "core.h"

template <typename T>
struct list;

template <typename T>
struct list_reverse_iterator;

template <typename T>
struct list_const_iterator;

template <typename T>
struct list_const_reverse_iterator;

template <typename T>
struct list_iterator
{
  list<T> *pList = nullptr;
  size_t position = 0;

  list_iterator(list<T> *pList);
  T &operator *();
  const T &operator *() const;
  bool operator != (const size_t maxCount) const;
  bool operator != (const list_iterator<T> &it) const;
  bool operator != (const list_reverse_iterator<T> &it) const;
  bool operator != (const list_const_iterator<T> &it) const;
  bool operator != (const list_const_reverse_iterator<T> &it) const;
  list_iterator<T> &operator++();
};

template <typename T>
struct list_reverse_iterator
{
  list<T> *pList = nullptr;
  int64_t position = 0;

  list_reverse_iterator(list<T> *pList);
  T &operator *();
  const T &operator *() const;
  bool operator != (const size_t startIndex) const;
  bool operator != (const list_iterator<T> &it) const;
  bool operator != (const list_reverse_iterator<T> &it) const;
  bool operator != (const list_const_iterator<T> &it) const;
  bool operator != (const list_const_reverse_iterator<T> &it) const;
  list_reverse_iterator<T> &operator++();
};

template <typename T>
struct list_const_iterator
{
  const list<T> *pList = nullptr;
  size_t position = 0;

  list_const_iterator(const list<T> *pList);
  const T &operator *() const;
  bool operator != (const size_t maxCount) const;
  bool operator != (const list_iterator<T> &it) const;
  bool operator != (const list_reverse_iterator<T> &it) const;
  bool operator != (const list_const_iterator<T> &it) const;
  bool operator != (const list_const_reverse_iterator<T> &it) const;
  list_const_iterator<T> &operator++();
};

template <typename T>
struct list_const_reverse_iterator
{
  const list<T> *pList = nullptr;
  int64_t position = 0;

  list_const_reverse_iterator(const list<T> *pList);
  const T &operator *() const;
  bool operator != (const size_t startIndex) const;
  bool operator != (const list_iterator<T> &it) const;
  bool operator != (const list_reverse_iterator<T> &it) const;
  bool operator != (const list_const_iterator<T> &it) const;
  bool operator != (const list_const_reverse_iterator<T> &it) const;
  list_const_reverse_iterator<T> &operator++();
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct list
{
  T *pValues = nullptr;
  size_t count = 0;
  size_t capacity = 0;

  inline list_iterator<T> begin() { return list_iterator<T>(this); };
  inline list_const_iterator<T> begin() const { return list_const_iterator<T>(this); };
  inline size_t end() const { return count; };

  inline struct
  {
    list<T> *pList;
    inline list_reverse_iterator<T> begin() { return list_reverse_iterator<T>(pList); };
    inline size_t end() { return 0; };
  } IterateReverse() { return { this }; };

  inline struct
  {
    const list<T> *pList;
    inline list_const_reverse_iterator<T> begin() { return list_const_reverse_iterator<T>(pList); };
    inline size_t end() { return 0; };
  } IterateReverse() const { return { this }; };

  inline struct
  {
    list<T> *pList;
    size_t startIdx;

    inline list_iterator<T> begin()
    {
      const list_iterator<T> it = list_iterator<T>(pList);
      it.position = startIdx;
      return it;
    };

    inline size_t end() { return count; };
  } IterateFrom(const size_t idx) { return { this, idx }; };

  inline struct
  {
    list<T> *pList;
    size_t startIdx;

    inline list_const_iterator<T> begin()
    {
      const list_const_iterator<T> it = list_const_iterator<T>(pList);
      it.position = startIdx;
      return it;
    };

    inline size_t end() { return count; };
  } IterateFrom(const size_t idx) const { return { this, idx }; };

  inline struct
  {
    list<T> *pList;
    size_t startIdx;

    inline list_reverse_iterator<T> begin()
    {
      const list_reverse_iterator<T> it = list_reverse_iterator<T>(pList);
      it.position = startIdx;
      return it;
    };

    inline size_t end() { return 0; };
  } IterateReverseFrom(const size_t idx) { return { this, idx }; };

  inline struct
  {
    list<T> *pList;
    size_t startIdx;

    inline list_const_reverse_iterator<T> begin()
    {
      const list_const_reverse_iterator<T> it = list_const_reverse_iterator<T>(pList);
      it.position = startIdx;
      return it;
    };

    inline size_t end() { return 0; };
  } IterateReverseFrom(const size_t idx) const { return { this, idx }; };

  T &operator [](const size_t index);
  const T &operator [](const size_t index) const;

  ~list();

  inline list() {};
  inline list(const list &) = delete;
  inline list &operator = (const list &) = delete;

  inline list(list &&move) :
    count(move.count),
    pValues(move.pValues),
    capacity(move.capacity)
  {
    move.pValues = nullptr;
    move.count = 0;
    move.capacity = 0;
  }

  inline list &operator = (list &&move)
  {
    list_destroy(this);

    count = move.count;
    pValues = move.pValues;
    capacity = move.capacity;

    move.pValues = nullptr;
    move.count = 0;
    move.capacity = 0;

    return *this;
  }

  template <typename ioif>
  static inline lsResult io(list<T> &self, ioif &context);
};

template <typename T>
lsResult list_reserve(list<T> *pList, const size_t count)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr, lsR_ArgumentNull);

  if (count > pList->capacity)
  {
    const size_t newCapacity = count;
    LS_ERROR_CHECK(lsRealloc(&pList->pValues, newCapacity));
    pList->capacity = newCapacity;
  }

epilogue:
  return result;
}

template <typename T>
lsResult list_reserve(list<T> &list, const size_t count)
{
  return list_reserve(&list, count);
}

template <typename T>
lsResult list_add(list<T> *pList, const T *pItem)
{
  lsResult result = lsR_Success;

  lsAssert(pList && pItem);

  const size_t extraIndex = pList->count;

  // Grow if we need to grow.
  if (pList->capacity <= extraIndex)
  {
    const size_t newCapacity = (pList->capacity * 2 + 64) & ~(size_t)(64 - 1);
    LS_ERROR_CHECK(lsRealloc(&pList->pValues, newCapacity));
    pList->capacity = newCapacity;
  }

  new (&pList->pValues[extraIndex]) T(*pItem);
  pList->count++;

epilogue:
  return result;
}

template <typename T>
lsResult list_add(list<T> *pList, T &&item)
{
  lsResult result = lsR_Success;

  lsAssert(pList);

  const size_t extraIndex = pList->count;

  // Grow if we need to grow.
  if (pList->capacity <= extraIndex)
  {
    const size_t newCapacity = (pList->capacity * 2 + 64) & ~(size_t)(64 - 1);
    LS_ERROR_CHECK(lsRealloc(&pList->pValues, newCapacity));
    pList->capacity = newCapacity;
  }

  new (&pList->pValues[extraIndex]) T(std::move(item));
  pList->count++;

epilogue:
  return result;
}

template <typename T>
lsResult list_add(list<T> *pList, const T &item)
{
  return list_add(pList, &item);
}

template <typename T>
lsResult list_add(list<T> &list, const T &item)
{
  return list_add(&list, &item);
}

template <typename T>
lsResult list_add(list<T> &list, T &&item)
{
  return list_add(&list, std::forward<T>(item));
}

template <typename T>
lsResult list_push_back(list<T> &list, const T &item)
{
  return list_add(&list, &item);
}

template <typename T>
lsResult list_push_back(list<T> &list, T &&item)
{
  return list_add(&list, std::forward<T>(item));
}

template <typename T>
lsResult list_add_range(list<T> *pList, const T *pItems, const size_t count)
{
  // TODO: FIXME: this is bugged somehow. possibly with T only being movable rather than copyable????

  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr || pItems == nullptr, lsR_ArgumentNull);

  // Grow if we need to grow.
  if (pList->count + count > pList->capacity)
  {
    const size_t requiredCapacity = pList->count + count;
    const size_t newCapacity = (lsMax(pList->capacity * 2, requiredCapacity) + 64) & ~(size_t)(64 - 1);
    LS_ERROR_CHECK(lsRealloc(&pList->pValues, newCapacity));
    pList->capacity = newCapacity;
  }

  lsMemcpy(pList->pValues + pList->count, pItems, count);
  pList->count += count;

epilogue:
  return result;
}

template <typename T>
const T *list_get(const list<T> *pList, const size_t index)
{
  return &pList->pValues[index];
}

template <typename T>
T *list_get(list<T> *pList, const size_t index)
{
  return &pList->pValues[index];
}

template <typename T>
lsResult list_get_safe(const list<T> *pList, const size_t index, _Out_ T *pItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr || pItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(index >= pList->count, lsR_ArgumentOutOfBounds);

  *pItem = pList->pValues[index];

epilogue:
  return result;
}

template <typename T>
lsResult list_get_safe(list<T> *pList, const size_t index, _Out_ T **ppItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr || ppItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(index >= pList->count, lsR_ArgumentOutOfBounds);

  *ppItem = &pList->pValues[index];

epilogue:
  return result;
}

template <typename T>
lsResult list_get_safe(const list<T> *pList, const size_t index, _Out_ T *const *ppItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr || ppItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(index >= pList->count, lsR_ArgumentOutOfBounds);

  *ppItem = &pList->pValues[index];

epilogue:
  return result;
}

template <typename T>
lsResult list_pop_back_safe(list<T> *pList, _Out_ T *pItem = nullptr)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(pList->count == 0, lsR_ArgumentOutOfBounds);

  if (pItem != nullptr)
    *pItem = pList->pValues[pList->count - 1];

  pList->count--;

epilogue:
  return result;
}

template <typename T>
T list_pop_back(list<T> &list)
{
  lsAssert(list.count > 0);

  list.count--;
  return std::move(list.pValues[list.count]);
}

template <typename T>
lsResult list_pop_front_safe(list<T> *pList, _Out_ T *pItem = nullptr)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(pList->count == 0, lsR_ArgumentOutOfBounds);

  if (pItem != nullptr)
    *pItem = std::move(pList->pValues[0]);

  pList->count--;

  if constexpr (std::is_trivially_move_constructible_v<T>)
  {
    lsMemmove(pList->pValues, pList->pValues + 1, pList->count);
  }
  else
  {
    for (size_t i = 0; i < pList->count; i++)
      new (&pList->pValues[i]) T(std::move(pList->pValues[i + 1]));
  }

epilogue:
  return result;
}

template <typename T>
T list_pop_front(list<T> &list)
{
  lsAssert(list.count > 0);

  T tmp = std::move(list.pValues[0]);
  list.count--;

  if constexpr (std::is_trivially_move_constructible_v<T>)
  {
    lsMemmove(list.pValues, list.pValues + 1, list.count);
  }
  else
  {
    for (size_t i = 0; i < list.count; i++)
      new (&list.pValues[i]) T(std::move(list.pValues[i + 1]));
  }

  return tmp;
}

template <typename T>
lsResult list_resize(list<T> *pList, const size_t newSize, const T *pDefaultItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pList == nullptr || pDefaultItem == nullptr, lsR_ArgumentNull);

  if (newSize > pList->count)
  {
    if (newSize > pList->capacity)
      LS_ERROR_CHECK(list_reserve(pList, newSize));

    for (size_t i = pList->count; i < newSize; i++)
      pList->pValues[i] = *pDefaultItem;
  }

  pList->count = newSize;

epilogue:
  return result;
}

template <typename T>
lsResult list_resize(list<T> *pList, const size_t newSize, const T &defaultItem)
{
  return list_resize(pList, newSize, &defaultItem);
}

template <typename T>
void list_clear(list<T> *pList)
{
  if (pList == nullptr)
    return;

  for (size_t i = 0; i < pList->count; i++)
    pList->pValues[i].~T();

  pList->count = 0;
}

template <typename T>
void list_destroy(list<T> *pList)
{
  if (pList == nullptr)
    return;

  for (size_t i = 0; i < pList->count; i++)
    pList->pValues[i].~T();

  lsFreePtr(&pList->pValues);

  pList->count = 0;
  pList->capacity = 0;
}

template <typename T, typename U>
T *list_contains(list<T> &list, const U &cmp)
{
  for (auto &_i : list)
    if (_i == cmp)
      return &_i;

  return nullptr;
}

template <typename T, typename U>
const T *list_contains(const list<T> &list, const U &cmp)
{
  for (auto &_i : list)
    if (_i == cmp)
      return &_i;

  return nullptr;
}

template <typename T, typename U>
T *list_contains(list<T> *pList, const U &cmp)
{
  lsAssert(pList != nullptr);
  return list_contains(*pList, cmp);
}

template <typename T, typename U>
const T *list_contains(const list<T> *pList, const U &cmp)
{
  lsAssert(pList != nullptr);
  return list_contains(*pList, cmp);
}

template<typename T, typename TLessFunc = std::less<T>, typename TGreaterFunc = std::greater<T>>
inline void list_sort(list<T> &l)
{
  struct _internal
  {
    static void dualPivotQuickSort_partition(list<T> &l, const int64_t low, const int64_t high, int64_t *pRightPivot, int64_t *pLeftPivot)
    {
      TLessFunc _less = TLessFunc();
      TGreaterFunc _greater = TGreaterFunc();

      if (_greater(l.pValues[low], l.pValues[high]))
        std::swap(l.pValues[low], l.pValues[high]);

      int64_t j = low + 1;
      int64_t g = high - 1;
      int64_t k = low + 1;

      T *pP = &l.pValues[low];
      T *pQ = &l.pValues[high];

      while (k <= g)
      {
        if (_less(l.pValues[k], *pP))
        {
          std::swap(l.pValues[k], l.pValues[j]);
          j++;
        }

        else if (!_less(l.pValues[k], *pQ))
        {
          while (_greater(l.pValues[g], *pQ) && k < g)
            g--;

          std::swap(l.pValues[k], l.pValues[g]);
          g--;

          if (_less(l.pValues[k], *pP))
          {
            std::swap(l.pValues[k], l.pValues[j]);
            j++;
          }
        }

        k++;
      }

      j--;
      g++;

      std::swap(l.pValues[low], l.pValues[j]);
      std::swap(l.pValues[high], l.pValues[g]);

      *pLeftPivot = j;
      *pRightPivot = g;
    }

    static void quickSort(list<T> &l, const int64_t start, const int64_t end)
    {
      if (start < end)
      {
        int64_t leftPivot, rightPivot;

        dualPivotQuickSort_partition(l, start, end, &rightPivot, &leftPivot);
        lsAssert(rightPivot < (int64_t)l.count);
        lsAssert(leftPivot < (int64_t)l.count);

        quickSort(l, start, leftPivot - 1);
        quickSort(l, leftPivot + 1, rightPivot - 1);
        quickSort(l, rightPivot + 1, end);
      }
    }
  };

  if (l.count)
    _internal::quickSort(l, 0, l.count - 1);
}

template<typename T, typename TLessFunc = std::less<T>, typename TGreaterFunc = std::greater<T>>
inline void list_sort_descending(list<T> &l)
{
  return list_sort<T, TGreaterFunc, TLessFunc>(l);
}

//////////////////////////////////////////////////////////////////////////

template <typename T, typename TGreater = std::greater<T>>
inline void list_to_heap_percolate_recursive(list<T> &l, const size_t idx, const size_t count)
{
  TGreater _greater;

  size_t largest = idx;
  const size_t lChild = idx * 2 + 1;
  const size_t rChild = idx * 2 + 2;

  if (lChild < count)
  {
    if (_greater(l[lChild], l.pValues[largest]))
      largest = lChild;
  }

  if (rChild < count)
  {
    if (_greater(l[rChild], l.pValues[largest]))
      largest = rChild;
  }

  if (largest != idx)
  {
    std::swap(l[idx], l[largest]);
    list_to_heap_percolate_recursive<T, TGreater>(l, largest, count);
  }
}

template <typename T, typename TGreater = std::greater<T>>
inline void list_to_heap(list<T> &l, const size_t count = (size_t)-1)
{
  size_t end = count;

  if (end == (size_t)-1)
    end = l.count;

  for (int64_t i = end / 2 - 1; i >= 0; i--)
    list_to_heap_percolate_recursive<T, TGreater>(l, i, end);
}

template <typename T, typename TGreater = std::greater<T>>
inline void list_heap_sort(list<T> &l)
{
  list_to_heap<T, TGreater>(l);

  for (int64_t i = l.count - 1; i >= 0; i--)
  {
    std::swap(l.pValues[0], l.pValues[i]);
    list_to_heap_percolate_recursive<T, TGreater>(l, 0, i);
  }
}

template <typename T, typename TLess = std::less<T>>
inline void list_heap_sort_descending(list<T> &l)
{
  list_heap_sort<T, TLess>(l);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
void list_remove_at(list<T> &l, const size_t index, _Out_opt_ T *pOut = nullptr)
{
  lsAssert(l.count > index);

  if (pOut == nullptr)
    l.pValues[index].~T();
  else
    new (pOut) T(std::move(l.pValues[index]));

  const size_t moveCount = l.count - index - 1;

  if constexpr (std::is_trivially_move_constructible_v<T>)
  {
    lsMemmove(l.pValues + index, l.pValues + index + 1, moveCount);
  }
  else
  {
    for (int64_t j = moveCount - 1; j >= 0; j--)
      new (l.pValues + index + j) T(std::move(l.pValues[index + 1 + j]));
  }

  l.count--;
}

template <typename T, typename TComparable, typename TEqual = std::equal_to<TComparable>>
inline bool list_remove_element(list<T> &l, const TComparable &element, T *pOut = nullptr)
{
  TEqual _equal;

  if (l.count == 0)
    return false;

  size_t index = (size_t)-1;

  for (const auto &_elem : l)
  {
    ++index;

    if constexpr (std::is_same_v<std::equal_to<TComparable>, TEqual> && !std::is_same_v<TComparable, T>) // workaround for comparing with a different non-comparable type.
    {
      if (_elem == element)
      {
        if (pOut == nullptr)
          list_remove_at(l, index);
        else
          *pOut = list_remove_at(l, index);

        return true;
      }
    }
    else
    {
      if (_equal(_elem, element))
      {
        if (pOut == nullptr)
          list_remove_at(l, index);
        else
          *pOut = list_remove_at(l, index);

        return true;
      }
    }
  }

  return false;
}

template <typename T, typename TComparable, typename TEqual = std::equal_to<TComparable>>
inline bool list_remove_all_matching_elements(list<T> &l, const TComparable &element)
{
  TEqual _equal;

  if (l.count == 0)
    return false;

  size_t index = l.count;
  bool found = false;

  for (const auto &_elem : l.IterateReverse())
  {
    --index;

    if constexpr (std::is_same_v<std::equal_to<TComparable>, TEqual> && !std::is_same_v<TComparable, T>) // workaround for comparing with a different non-comparable type.
    {
      if (_elem == element)
      {
        list_remove_at(l, index);
        found = true;
      }
    }
    else
    {
      if (_equal(_elem, element))
      {
        list_remove_at(l, index);
        found = true;
      }
    }
  }

  return found;
}

//////////////////////////////////////////////////////////////////////////

// Return index of first element for which l[i] < cmp is false or endIdx + 1 if for none.
// See: https://en.cppreference.com/w/cpp/algorithm/lower_bound
template <typename T, typename TCompare, typename TLessFunc = std::less<T>>
inline size_t sorted_list_find_lower_bound(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  TLessFunc less;

  size_t first = startIdx;
  const size_t last = endIdx == (size_t)-1 ? l.count : (endIdx + 1);
  int64_t count = (int64_t)(last - first);

  while (count > 0)
  {
    const size_t step = count / 2;
    const size_t i = first + step;

    if (less(l[i], cmp))
    {
      first = i + 1;
      count -= step + 1;
    }
    else
    {
      count = step;
    }
  }

  return first;
}

template <typename T, typename TCompare, typename TGreaterFunc = std::greater<T>>
inline size_t descending_sorted_list_find_lower_bound(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  return sorted_list_find_lower_bound<T, TCompare, TGreaterFunc>(l, cmp, startIdx, endIdx);
}

// Return index of first element for which cmp < l[i] is true or endIdx + 1 if for none.
// See: https://en.cppreference.com/w/cpp/algorithm/upper_bound
template <typename T, typename TCompare, typename TLessFunc = std::less<T>>
inline size_t sorted_list_find_upper_bound(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  TLessFunc less;

  size_t first = startIdx;
  const size_t last = endIdx == (size_t)-1 ? l.count : (endIdx + 1);
  int64_t count = (int64_t)(last - first);

  while (count > 0)
  {
    const size_t step = count / 2;
    const size_t it = first += step;

    if (!less(cmp, l[it]))
    {
      first = it + 1;
      count -= step + 1;
    }
    else
    {
      count = step;
    }
  }

  return first;
}

template <typename T, typename TCompare, typename TGreaterFunc = std::greater<T>>
inline size_t descending_sorted_list_find_upper_bound(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  return sorted_list_find_upper_bound<T, TCompare, TGreaterFunc>(l, cmp, startIdx, endIdx);
}

template <typename T, typename TCompare, typename TLessFunc = std::less<T>, typename TEqualsFunc = std::equal_to<T>>
inline size_t sorted_list_find(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  const size_t idx = sorted_list_find_lower_bound<T, TCompare, TLessFunc>(l, cmp, startIdx, endIdx);

  if (idx == l.count)
    return (size_t)-1;

  TEqualsFunc equals;

  if (equals(l.pValues[idx], cmp))
    return idx;

  return (size_t)-1;
}

template <typename T, typename TCompare, typename TGreaterFunc = std::greater<T>, typename TEqualsFunc = std::equal_to<T>>
inline size_t descending_sorted_list_find(list<T> &l, const TCompare &cmp, const size_t startIdx = 0, const size_t endIdx = (size_t)-1)
{
  return sorted_list_find<T, TCompare, TGreaterFunc, TEqualsFunc>(l, cmp, startIdx, endIdx);
}

//////////////////////////////////////////////////////////////////////////

template <typename T, typename TIndex, bool OutputData>
  requires (std::is_integral_v<TIndex>)
void _list_remove_sorted_indexes_internal(list<T> &l, const TIndex *pIndexes, const size_t count, _Out_opt_ T *pOut = nullptr)
{
  lsAssert(l.count >= count);
  lsAssert(pIndexes != nullptr);

  for (size_t i = 0; i < count; i++)
  {
    const size_t oldIdx = pIndexes[i];
    const size_t newIdx = oldIdx - i;
    lsAssert(oldIdx >= i);
    lsAssert(oldIdx - i < l.count - i);

    if constexpr (!OutputData)
      l.pValues[oldIdx].~T();
    else
      new (&pOut[i]) T(std::move(l.pValues[oldIdx]));

    size_t moveCount;

    if (i + 1 == count)
    {
      moveCount = l.count - oldIdx - 1;
    }
    else
    {
      lsAssert(pIndexes[i + 1] >= i + 1);
      lsAssert(pIndexes[i + 1] - i - 1 >= newIdx);

      moveCount = pIndexes[i + 1] - i - 1 - newIdx;
    }

    if constexpr (std::is_trivially_move_constructible_v<T>)
    {
      lsMemmove(l.pValues + newIdx, l.pValues + oldIdx + 1, moveCount);
    }
    else
    {
      for (int64_t j = moveCount - 1; j >= 0; j--)
        new (l.pValues + newIdx + j) T(std::move(l.pValues[oldIdx + 1 + j]));
    }
  }

  l.count -= count;
}

template <typename T, typename TIndex>
  requires (std::is_integral_v<TIndex>)
void list_remove_sorted_indexes(list<T> &l, const TIndex *pIndexes, const size_t count, _Out_opt_ T *pOut = nullptr)
{
  if (pOut == nullptr)
    _list_remove_sorted_indexes_internal<T, TIndex, false>(l, pIndexes, count, pOut);
  else
    _list_remove_sorted_indexes_internal<T, TIndex, true>(l, pIndexes, count, pOut);
}

template <typename T, typename TIndex, typename TPtr, bool Move>
  requires (std::is_integral_v<TIndex>)
lsResult _list_insert_sorted_indexes_internal(list<T> &l, const TIndex *pIndexes, const size_t count, TPtr pValues)
{
  lsResult result = lsR_Success;

  lsAssert(pIndexes != nullptr || count == 0);
  lsAssert(pValues != nullptr || count == 0);

  if (l.count + count > l.capacity)
    LS_ERROR_CHECK(list_reserve(&l, l.count + count));

  if (count > 0)
  {
    const size_t nextOldIdx = pIndexes[count - 1];
    const size_t moveCount = l.count - nextOldIdx;

    if constexpr (std::is_trivially_move_constructible_v<T>)
    {
      lsMemmove(l.pValues + nextOldIdx + count, l.pValues + nextOldIdx, moveCount);
    }
    else
    {
      for (int64_t j = moveCount - 1; j >= 0; j--)
        new (l.pValues + nextOldIdx + count + j) T(std::move(l.pValues[nextOldIdx + j]));
    }
  }

  for (int64_t i = count - 1; i >= 0; i--)
  {
    const size_t oldIdx = pIndexes[i];
    const size_t newIdx = oldIdx + i;

    if constexpr (Move)
      new (l.pValues + newIdx) T(std::move(pValues[i]));
    else
      l.pValues[newIdx] = pValues[i];

    if (i > 0)
    {
      const size_t nextOldtIdx = pIndexes[i - 1];
      lsAssert(nextOldtIdx <= oldIdx);
      const size_t moveCount = oldIdx - nextOldtIdx;

      if constexpr (std::is_trivially_move_constructible_v<T>)
      {
        lsMemmove(l.pValues + newIdx - moveCount, l.pValues + newIdx - moveCount - i, moveCount);
      }
      else
      {
        for (size_t j = 0; j < moveCount; j++)
          new (l.pValues + newIdx - moveCount) T(std::move(l.pValues[newIdx - moveCount - i + j]));
      }
    }
  }

  l.count += count;

epilogue:
  return result;
}

template <typename T, typename TIndex>
  requires (std::is_integral_v<TIndex>)
lsResult list_insert_sorted_indexes_move(list<T> &l, const TIndex *pIndexes, const size_t count, T *pValues)
{
  return _list_insert_sorted_indexes_internal<T, TIndex, T *, true>(l, pIndexes, count, pValues);
}

template <typename T, typename TIndex>
  requires (std::is_integral_v<TIndex>)
lsResult list_insert_sorted_indexes(list<T> &l, const TIndex *pIndexes, const size_t count, const T *pValues)
{
  return _list_insert_sorted_indexes_internal<T, TIndex, const T *, false>(l, pIndexes, count, pValues);
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline T &list<T>::operator[](const size_t index)
{
  return *list_get(this, index);
}

template<typename T>
inline const T &list<T>::operator[](const size_t index) const
{
  return *list_get(this, index);
}

template<typename T>
inline list<T>::~list()
{
  list_destroy(this);
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline list_iterator<T>::list_iterator(list<T> *pList) :
  pList(pList)
{
}

template<typename T>
inline T &list_iterator<T>::operator*()
{
  return pList->pValues[position];
}

template<typename T>
inline const T &list_iterator<T>::operator*() const
{
  return pList->pValues[position];
}

template<typename T>
inline bool list_iterator<T>::operator!=(const size_t maxCount) const
{
  return position < maxCount;
}

template<typename T>
inline bool list_iterator<T>::operator!=(const list_iterator<T> &it) const
{
  return position < it.position;
}

template<typename T>
inline bool list_iterator<T>::operator!=(const list_reverse_iterator<T> &it) const
{
  return (int64_t)position < it.position;
}

template<typename T>
inline bool list_iterator<T>::operator!=(const list_const_iterator<T> &it) const
{
  return position < it.position;
}

template<typename T>
inline bool list_iterator<T>::operator!=(const list_const_reverse_iterator<T> &it) const
{
  return (int64_t)position < it.position;
}

template<typename T>
inline list_iterator<T> &list_iterator<T>::operator++()
{
  position++;

  return *this;
}

template<typename T>
inline list_reverse_iterator<T>::list_reverse_iterator(list<T> *pList) :
  pList(pList),
  position((size_t)lsMax((int64_t)0, (int64_t)pList->count - 1))
{
}

template<typename T>
inline T &list_reverse_iterator<T>::operator*()
{
  return pList->pValues[position];
}

template<typename T>
inline const T &list_reverse_iterator<T>::operator*() const
{
  return pList->pValues[position];
}

template<typename T>
inline bool list_reverse_iterator<T>::operator!=(const size_t startIndex) const
{
  return position >= (int64_t)startIndex;
}

template<typename T>
inline bool list_reverse_iterator<T>::operator!=(const list_iterator<T> &it) const
{
  return position >= (int64_t)it.position;
}

template<typename T>
inline bool list_reverse_iterator<T>::operator!=(const list_reverse_iterator<T> &it) const
{
  return position >= it.position;
}

template<typename T>
inline bool list_reverse_iterator<T>::operator!=(const list_const_iterator<T> &it) const
{
  return position >= (int64_t)it.position;
}

template<typename T>
inline bool list_reverse_iterator<T>::operator!=(const list_const_reverse_iterator<T> &it) const
{
  return position >= it.position;
}

template<typename T>
inline list_reverse_iterator<T> &list_reverse_iterator<T>::operator++()
{
  position--;

  return *this;
}

template<typename T>
inline list_const_iterator<T>::list_const_iterator(const list<T> *pList) :
  pList(pList)
{
}

template<typename T>
inline const T &list_const_iterator<T>::operator*() const
{
  return pList->pValues[position];
}

template<typename T>
inline bool list_const_iterator<T>::operator!=(const size_t maxCount) const
{
  return position < maxCount;
}

template<typename T>
inline bool list_const_iterator<T>::operator!=(const list_iterator<T> &it) const
{
  return position < it.position;
}

template<typename T>
inline bool list_const_iterator<T>::operator!=(const list_reverse_iterator<T> &it) const
{
  return (int64_t)position < it.position;
}

template<typename T>
inline bool list_const_iterator<T>::operator!=(const list_const_iterator<T> &it) const
{
  return position < it.position;
}

template<typename T>
inline bool list_const_iterator<T>::operator!=(const list_const_reverse_iterator<T> &it) const
{
  return (int64_t)position < it.position;
}

template<typename T>
inline list_const_iterator<T> &list_const_iterator<T>::operator++()
{
  position++;

  return *this;
}

template<typename T>
inline list_const_reverse_iterator<T>::list_const_reverse_iterator(const list<T> *pList) :
  pList(pList),
  position((size_t)lsMax((int64_t)0, (int64_t)pList->count - 1))
{
}

template<typename T>
inline const T &list_const_reverse_iterator<T>::operator*() const
{
  return pList->pValues[position];
}

template<typename T>
inline bool list_const_reverse_iterator<T>::operator!=(const size_t startIndex) const
{
  return position >= (int64_t)startIndex;
}

template<typename T>
inline bool list_const_reverse_iterator<T>::operator!=(const list_iterator<T> &it) const
{
  return position >= (int64_t)it.position;
}

template<typename T>
inline bool list_const_reverse_iterator<T>::operator!=(const list_reverse_iterator<T> &it) const
{
  return position >= it.position;
}

template<typename T>
inline bool list_const_reverse_iterator<T>::operator!=(const list_const_iterator<T> &it) const
{
  return position >= (int64_t)it.position;
}

template<typename T>
inline bool list_const_reverse_iterator<T>::operator!=(const list_const_reverse_iterator<T> &it) const
{
  return position >= it.position;
}

template<typename T>
inline list_const_reverse_iterator<T> &list_const_reverse_iterator<T>::operator++()
{
  position--;

  return *this;
}
