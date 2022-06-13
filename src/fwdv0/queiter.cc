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

#include <fwdv0/que.h>
#include <cstddef>
#include <stdexcept>

namespace fwdv0 {

  // Ctor/Dtor/Copy

  template <bool Const>
  QueBase::IterBase<Const>::IterBase() {}

  template <bool Const>
  QueBase::IterBase<Const>::IterBase(container_type pParent, chunk_type pChunk, size_t idx) : m_pParent{pParent}, m_pChunk{pChunk}, m_idx{idx} {}

  template <bool Const>
  QueBase::IterBase<Const>::~IterBase() {}

  template <bool Const>
  QueBase::IterBase<Const>::IterBase(const IterBase& rhs) = default;

  template <bool Const>
  QueBase::IterBase<Const>& QueBase::IterBase<Const>::operator=(const IterBase& rhs) = default;


  // Comparison and incr/decr

  template <bool Const>
  bool QueBase::IterBase<Const>::operator!=(const IterBase<Const>& rhs) const {
    return rhs.m_pParent != m_pParent || rhs.m_pChunk != m_pChunk || rhs.m_idx != m_idx;
  }

  template <bool Const>
  bool QueBase::IterBase<Const>::operator==(const IterBase<Const>& rhs) const {
    return !(*this != rhs);
  }

  template <bool Const>
  QueBase::IterBase<Const>& QueBase::IterBase<Const>::operator++() {
    if (m_idx == m_pParent->m_nPerChunk-1) {
      m_idx = 0;
      m_pChunk = m_pChunk->pNext;
    } else {
      ++m_idx;
    }
    return *this;
  }

  template <bool Const>
  QueBase::IterBase<Const> QueBase::IterBase<Const>::operator++(int) {
    QueBase::IterBase rtn {*this};
    ++(*this);
    return rtn;
  }

  template <bool Const>
  QueBase::IterBase<Const>& QueBase::IterBase<Const>::operator--() {
    if (m_idx == 0) {
      m_idx = m_pParent->m_nPerChunk-1;
      m_pChunk = m_pChunk->pPrev;
    } else {
      --m_idx;
    }
    return *this;
  }

  template <bool Const>
  QueBase::IterBase<Const> QueBase::IterBase<Const>::operator--(int) {
    QueBase::IterBase rtn {*this};
    --(*this);
    return rtn;
  }

  // Element Access

  template <bool Const>
  char const* QueBase::IterBase<Const>::getPtr() const {
    return m_pParent->getElementPointerInChunk(m_pChunk, m_idx);
  }

  template <bool Const>
  char* QueBase::IterBase<Const>::getPtr() {
    return const_cast<char*>(std::as_const(*this).getPtr());
  }

  // Construction from the container

  QueBase::IterBase<true> QueBase::beginBase() const {
    return IterBase<true>{this, m_pChunkFront, m_offset};
  }

  QueBase::IterBase<false> QueBase::beginBase() {
    return IterBase<false>{this, m_pChunkFront, m_offset};
  }

  QueBase::IterBase<true> QueBase::endBase() const {
    auto id = getIdInChunk(m_size);
    if (m_size > 0 && id == 0) return IterBase<true>{this, nullptr, 0};
    return IterBase<true>{this, m_pChunkBack, id};
  }

  QueBase::IterBase<false> QueBase::endBase() {
    auto id = getIdInChunk(m_size);
    if (m_size > 0 && id == 0) return IterBase<false>{this, nullptr, 0};
    return IterBase<false>{this, m_pChunkBack, id};
  }


  // Explicit instantiation
  template struct QueBase::IterBase<true>;
  template struct QueBase::IterBase<false>;

}
