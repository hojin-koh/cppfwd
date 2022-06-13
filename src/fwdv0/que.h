/*
 * Copyright 2021-2022, Hojin Koh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Restricted unrolled linked list, pretending to be a deque-like data structure
#pragma once
#include <type_traits>
#include <utility>

namespace fwdv0 {

  using size_t = decltype(sizeof(0));
  using ssize_t = std::make_signed<size_t>::type;

  namespace {
    constexpr size_t CPPFWDV0_QUE_CHUNK_SIZE = 2048;
    constexpr size_t CPPFWDV0_QUE_MAX_ELEM_PER_CHUNK = 128;
    constexpr size_t CPPFWDV0_QUE_MIN_ELEM_PER_CHUNK = 4;
  }

  // The dirty-hack data chunk. We never directly delete or new using this data structure.
  // Instead, a properly-aligned char[] memory block is allocated and then this structure is projected onto that
  struct QueChunk {
    QueChunk *pNext, *pPrev;
    alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__) char data[1];
  };

  // The base class responsible for manipulating memory
  struct QueBase {
    QueBase(size_t szElem, size_t nPerChunk) : m_szElem{szElem}, m_nPerChunk{nPerChunk} {};
    QueBase(QueBase&& rhs);
    ~QueBase();

    // Public Queries
    size_t size() const;

    // iterator
    template <bool Const>
    struct IterBase {
      using container_type = typename std::conditional_t<Const, QueBase const*, QueBase*>;
      using chunk_type = typename std::conditional_t<Const, QueChunk const*, QueChunk*>;
      IterBase();
      IterBase(container_type pParent, chunk_type pChunk, size_t m_idx);
      ~IterBase();

      IterBase(const IterBase<Const>& rhs);
      IterBase& operator=(const IterBase<Const>& rhs);

      bool operator==(const IterBase<Const>& rhs) const;
      bool operator!=(const IterBase<Const>& rhs) const;
      IterBase& operator++();
      IterBase operator++(int);
      IterBase& operator--();
      IterBase operator--(int);

      char const* getPtr() const;
      char* getPtr();
    private:
      container_type m_pParent;
      chunk_type m_pChunk;
      size_t m_idx;
    };
    IterBase<true> beginBase() const;
    IterBase<false> beginBase();
    IterBase<true> endBase() const;
    IterBase<false> endBase();


  protected:
    const size_t m_szElem; // byte-size of each element
    const size_t m_nPerChunk; // # of elements inside a chunk

    QueChunk *m_pChunkFront {nullptr}, *m_pChunkBack {nullptr};
    size_t m_size {0}; // total # of elements
    size_t m_offset {0}; // element offset in the first chunk

    // Queries - Bound checks are enforced inside here
    char const* getElement(size_t i) const;
    char* getElement(size_t i);

    // Insertion
    char* addElementBack();
    char* addElementFront();

  private:
    // Chunk Queries
    size_t getIdInChunk(size_t i) const;
    size_t getChunkIdByElement(size_t i) const;
    QueChunk const* getChunkByElement(size_t i) const;
    QueChunk* getChunkByElement(size_t i);
    char const* getElementPointerInChunk(QueChunk const* p, size_t i) const;
    char* getElementPointerInChunk(QueChunk* p, size_t i);

    // Memory manipulation
    QueChunk* addInitialChunk();
    QueChunk* newChunk(QueChunk* pPrev, QueChunk* pNext);
    void deleteChunk(QueChunk* p);
  };


  template <typename T>
  struct Que final : public QueBase {
    constexpr static size_t s_szElem {sizeof(T)};
    // Decide (by heuristic) how many element we want inside one chunk
    constexpr static size_t s_nPerChunk {
      (CPPFWDV0_QUE_CHUNK_SIZE / s_szElem > CPPFWDV0_QUE_MAX_ELEM_PER_CHUNK) ?
        CPPFWDV0_QUE_MAX_ELEM_PER_CHUNK :
        ((CPPFWDV0_QUE_CHUNK_SIZE / s_szElem < CPPFWDV0_QUE_MIN_ELEM_PER_CHUNK) ?
         CPPFWDV0_QUE_MIN_ELEM_PER_CHUNK :
         CPPFWDV0_QUE_CHUNK_SIZE / s_szElem
        )
    };

    // Constructor
    Que() : QueBase(s_szElem, s_nPerChunk) {}
  //  Que(size_t n, const T& val); // fill constructor
  //  // range?
  //  // initializer?

    Que(Que<T>&& rhs) : QueBase(std::move(rhs)) {}

    Que(const Que<T>& rhs) : QueBase(s_szElem, s_nPerChunk) {
      for (const auto& obj : rhs) {
        push_back(obj);
      }
    }

    // The actually release of memory blocks is done by ~QueBase()
    ~Que() {
      for (const auto& obj : *this) {
        obj.~T();
      }
    }

  //  // copy/move etc.

    // === Access ===
    T& at(ssize_t i) {
      if (i<0) i = m_size + i;
      return *reinterpret_cast<T*>(getElement(i));
    }

    T const& at(ssize_t i) const {
      if (i<0) i = m_size + i;
      return *reinterpret_cast<T const*>(getElement(i));
    }

    T& operator[](ssize_t i) {
      if (i<0) i = m_size + i;
      return *reinterpret_cast<T*>(getElement(i));
    }

    T const& operator[](ssize_t i) const {
      if (i<0) i = m_size + i;
      return *reinterpret_cast<T const*>(getElement(i));
    }

    template <bool Const>
    struct Iter {
      Iter() {};
      Iter(const IterBase<Const>& itr) : m_itr{itr} {};
      Iter(IterBase<Const>&& itr) : m_itr{std::move(itr)} {};
      ~Iter() {};

      Iter(const Iter& rhs) = default;
      Iter& operator=(const Iter& rhs) = default;

      bool operator==(const Iter& rhs) const { return rhs.m_itr == this->m_itr;}
      bool operator!=(const Iter& rhs) const { return rhs.m_itr != this->m_itr;}
      Iter& operator++() { m_itr++; return *this; }
      Iter operator++(int) { auto rtn = *this; m_itr++; return rtn; }
      Iter& operator--() { m_itr--; return *this; }
      Iter operator--(int) { auto rtn = *this; m_itr--; return rtn; }

      T const& operator*() const { return *(reinterpret_cast<T const*>(m_itr.getPtr())); }
      T const* operator->() const { return reinterpret_cast<T const*>(m_itr.getPtr()); }

    private:
      IterBase<Const> m_itr;
    };

    using iterator = Iter<false>;
    using const_iterator = Iter<true>;

    Iter<true> begin() const {
      return Iter<true>{beginBase()};
    }

    Iter<false> begin() {
      return Iter<false>{beginBase()};
    }

    Iter<true> end() const {
      return Iter<true>{endBase()};
    }

    Iter<false> end() {
      return Iter<false>{endBase()};
    }

    Iter<true> cbegin() const {
      return Iter<true>{beginBase()};
    }

    Iter<true> cend() const {
      return Iter<true>{endBase()};
    }

    // === Insertion ===
    template<typename... ARGS>
    T& emplace_back(ARGS&&... args) {
      T* p = new (addElementBack()) T{std::forward<ARGS>(args)...};
      return *p;
    }

    T& push_back(const T& obj) {
      T* p = new (addElementBack()) T{obj};
      return *p;
    }

    template<typename... ARGS>
    T& emplace_front(ARGS&&... args) {
      T* p = new (addElementFront()) T{std::forward<ARGS>(args)...};
      return *p;
    }

    T& push_front(const T& obj) {
      T* p = new (addElementFront()) T{obj};
      return *p;
    }
  };

}

