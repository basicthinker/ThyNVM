/*
 * Copyright (c) 2014 Jinglei Ren <jinglei.ren@persper.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __THYNVM_VERSION_BUFFER_HH__
#define __THYNVM_VERSION_BUFFER_HH__

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>
#include "profiler.hh"

namespace thynvm {

class VersionBuffer
{
  public:
    enum State
    {
        SHORT = 0,
        LONG,
        IN_USE,
        FREE,  // max queue index
    };

    VersionBuffer(int length, int block_bits);

    uint64_t allocSlot(Profiler& profiler);
    void freeSlot(uint64_t hw_addr, State state, Profiler& profiler);
    void backupSlot(uint64_t hw_addr, State state, Profiler& profiler);
    void clearBackup(Profiler& profiler);

    uint64_t addrBase() const { return _addr_base; }
    void setAddrBase(uint64_t base) { _addr_base = base; }

    /**
     * Returns the number of block slots in this buffer area
     */
    int length() const { return _length; }

    /**
     * Returns the address space size that this buffer area covers in bytes
     */
    uint64_t size() const;

    bool contains(uint64_t addr) const;

  private:
    uint64_t at(int index);
    int index(uint64_t hw_addr);

    uint64_t _addr_base;
    const int _length;
    const int block_bits;
    std::vector<std::set<int>> index_sets;
};

inline
VersionBuffer::VersionBuffer(int length, int block_bits)
        : _length(length), block_bits(block_bits), index_sets(FREE + 1)
{
    for (int i = 0; i < _length; ++i) {
        index_sets[FREE].insert(i);
    }
    _addr_base = -EINVAL;
}

inline uint64_t
VersionBuffer::size() const
{
    return _length << block_bits;
}

inline bool
VersionBuffer::contains(uint64_t addr) const
{
    return addr >= _addr_base && (addr - _addr_base) < size();
}

inline uint64_t
VersionBuffer::at(int index)
{
    assert(_addr_base != -EINVAL && index >= 0 && index < _length);
    return _addr_base + (index << block_bits);
}

inline int
VersionBuffer::index(uint64_t hw_addr)
{
    assert(hw_addr >= _addr_base);
    uint64_t bytes = hw_addr - _addr_base;
    int i = bytes >> block_bits;
    assert(i >= 0 && i < _length);
    return i;
}

}  // namespace thynvm

#endif  // __THYNVM_VERSION_BUFFER_HH__
