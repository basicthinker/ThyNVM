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

#ifndef __THYNVM_ADDR_TRANS_TABLE_HH__
#define __THYNVM_ADDR_TRANS_TABLE_HH__

#include <cstdint>
#include <vector>
#include <unordered_map>
#include "base/index_queue.hh"
#include "profiler.hh"

namespace thynvm {

typedef int64_t Tag; // never negative
typedef uint64_t Addr;

struct ATTEntry
{
    enum State
    {
        FREE = 0,
        CLEAN,
        DIRTY,
        HIDDEN,
        PRE_HIDDEN,
        PRE_DIRTY,
        LOAN, // max queue index
    };

    Tag phy_tag;
    Addr hw_addr;
    State state;

    /**
     * Since AddrTransTable (ATT) is organized as an index array,
     * the IndexNode structure is embedded into each ATT entry.
     */
    IndexNode queue_node;

    // Statistics
    int epoch_reads;
    int epoch_writes;

    ATTEntry() : phy_tag(0), hw_addr(0), state(FREE),
            epoch_reads(0), epoch_writes(0) { }
};

class AddrTransTable: public IndexArray
{
  public:
    AddrTransTable(int length, int unit_bits);

    int insert(Tag phy_tag, Addr hw_addr, ATTEntry::State state,
            Profiler& profiler);
    int lookup(Tag phy_tag, Profiler& profiler);
    void shiftState(int index, ATTEntry::State state, Profiler& profiler);
    void reset(int index, Addr new_base, ATTEntry::State new_state,
            Profiler& profiler);

    bool isEmpty(ATTEntry::State state) const;
    bool contains(Addr phy_addr, Profiler& profiler) const;
    const ATTEntry& at(int i) const;

    int visitQueue(ATTEntry::State state, QueueVisitor* visitor);
    int getLength(ATTEntry::State state) const;
    int getFront(ATTEntry::State state) const;

    int length() const { return _length; }
    int unitSize() const { return 1 << unitBits; }

    Tag toTag(Addr addr) const { return Tag(addr >> unitBits); }
    Addr toAddr(Tag tag) const { return Addr(tag) << unitBits; }
    Addr toHardwareAddr(Addr phy_addr, Addr hw_base) const;

    void addReadCount(int index) { ++entries[index].epoch_reads; }
    void addWriteCount(int index) { ++entries[index].epoch_writes; }
    const std::vector<ATTEntry>& collectEntries() const { return entries; }
    void clearStats(Profiler& profiler);

private:
    IndexNode& operator[](int i) { return entries[i].queue_node; }

    const int _length;
    const int unitBits;
    const Addr unitMask;
    std::unordered_map<Tag, int> tagIndex;
    std::vector<ATTEntry> entries;
    std::vector<IndexQueue> queues;
};

inline
AddrTransTable::AddrTransTable(int length, int unit_bits)
        : _length(length), unitBits(unit_bits), unitMask(unitSize() - 1),
          entries(_length), queues(ATTEntry::LOAN + 1, *this)
{
    for (int i = 0; i < _length; ++i) {
        queues[ATTEntry::FREE].pushBack(i);
    }
}

inline const ATTEntry&
AddrTransTable::at(int i) const
{
    assert(i >= 0 && i < _length);
    return entries[i];
}

inline bool
AddrTransTable::contains(Addr phy_addr, Profiler& profiler) const
{
    profiler.addTableOp();
    return tagIndex.find(Tag(phy_addr)) != tagIndex.end();
}

inline int
AddrTransTable::visitQueue(ATTEntry::State state, QueueVisitor* visitor)
{
    assert(state < ATTEntry::LOAN);
    return queues[state].accept(visitor);
}

inline bool
AddrTransTable::isEmpty(ATTEntry::State state) const
{
    assert(state < ATTEntry::LOAN);
    return queues[state].empty();
}

inline int
AddrTransTable::getLength(ATTEntry::State state) const
{
    assert(state < ATTEntry::LOAN);
    return queues[state].length();
}

inline int
AddrTransTable::getFront(ATTEntry::State state) const
{
    assert(state < ATTEntry::LOAN);
    return queues[state].front();
}

inline Addr
AddrTransTable::toHardwareAddr(Addr phy_addr, Addr hw_base) const
{
    assert((hw_base & unitMask) == 0);
    return hw_base + (phy_addr & unitMask);
}

}  // namespace thynvm

#endif  // __THYNVM_ADDR_TRANS_TABLE_HH__
