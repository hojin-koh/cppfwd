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
#include <new>

namespace fwdv0 {

  // === Public Queries ===

  size_t QueBase::size() const {
    return m_size;
  }

  // === Queries ===

  char const* QueBase::getElement(size_t i) const {
    // TODO: use sds + format for the message here
    if (i >= m_size) throw std::out_of_range("Bound check failed on Que");
    return getElementPointerInChunk(getChunkByElement(i), getIdInChunk(i));
  }

  char* QueBase::getElement(size_t i) {
    return const_cast<char*>(std::as_const(*this).getElement(i));
  }

  // === Insertion ===

  char* QueBase::addElementBack() {
    QueChunk* p = m_pChunkBack;
    if (p == nullptr) { // The initial case
      p = addInitialChunk();
    }
    if (m_size == 0) {
      m_size++;
      return getElementPointerInChunk(p, 0);
    }

    // If not the empty case
    size_t iLast = getIdInChunk(m_size-1);
    m_size++;
    if (iLast == m_nPerChunk-1) { // The beginning of a new chunk
      p = m_pChunkBack = newChunk(p, nullptr);
      return getElementPointerInChunk(p, 0);
    }
    return getElementPointerInChunk(p, iLast+1);
  }

  char* QueBase::addElementFront() {
    QueChunk* p = m_pChunkFront;
    if (p == nullptr) { // The initial case
      p = addInitialChunk();
    }
    if (m_size == 0) {
      m_size++;
      m_offset = m_nPerChunk-1;
      return getElementPointerInChunk(p, m_nPerChunk-1);
    }

    // If not the empty case
    size_t iFirst = getIdInChunk(0);
    m_size++;
    if (m_offset == 0) { // The ending of a new chunk
      p = m_pChunkFront = newChunk(nullptr, p);
      m_offset = m_nPerChunk-1;
      return getElementPointerInChunk(p, m_nPerChunk-1);
    }
    --m_offset;
    return getElementPointerInChunk(p, m_offset);
  }

  // === Chunk Queries: no bound checks here ===

  size_t QueBase::getIdInChunk(size_t i) const {
    return (i+m_offset) % m_nPerChunk;
  }

  size_t QueBase::getChunkIdByElement(size_t i) const {
    return (i+m_offset) / m_nPerChunk;
  }

  QueChunk const* QueBase::getChunkByElement(size_t i) const {
    size_t idChunkBack = getChunkIdByElement(m_size-1);
    size_t idChunk = getChunkIdByElement(i);

    // First decide where we want to start the traversal
    // Then actually try to reach the requested chunk
    QueChunk* p;
    if (idChunk <= idChunkBack/2) { // Search from front
      p = m_pChunkFront;
      for (auto i=0; i<idChunk; ++i) p = p->pNext;
    } else { // Search from back
      p = m_pChunkBack;
      for (auto i=0; i<idChunkBack-idChunk; ++i) p = p->pPrev;
    }

    return p;
  }

  QueChunk* QueBase::getChunkByElement(size_t i) {
    return const_cast<QueChunk*>(std::as_const(*this).getChunkByElement(i));
  }

  char const* QueBase::getElementPointerInChunk(QueChunk const* p, size_t i) const {
    return p->data + i*m_szElem;
  }

  char* QueBase::getElementPointerInChunk(QueChunk* p, size_t i) {
    return p->data + i*m_szElem;
  }

  // === Memory manipulation ===

  QueChunk* QueBase::addInitialChunk() {
    m_pChunkFront = m_pChunkBack = newChunk(nullptr, nullptr);
    return m_pChunkFront;
  }

  // Allocate
  QueChunk* QueBase::newChunk(QueChunk* pPrev, QueChunk* pNext) {
    static constexpr size_t overhead {offsetof(QueChunk, data)};
    const size_t szAllocate {m_szElem*m_nPerChunk+overhead};
    QueChunk* p {reinterpret_cast<QueChunk*>(new (std::align_val_t{alignof(QueChunk)}) char[szAllocate])};
    p->pPrev = pPrev;
    if (pPrev != nullptr) pPrev->pNext = p;
    p->pNext = pNext;
    if (pNext != nullptr) pNext->pPrev = p;
    return p;
  }


}
