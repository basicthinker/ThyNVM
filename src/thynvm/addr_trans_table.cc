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

#include "addr_trans_table.hh"

#include <cerrno>

using namespace std;
using namespace thynvm;

int
AddrTransTable::lookup(Tag phy_tag, Profiler& profiler)
{
    profiler.addTableOp();
    unordered_map<Tag, int>::iterator it = tagIndex.find(phy_tag);
    if (it == tagIndex.end()) { // not hit
        return -EINVAL;
    } else {
        ATTEntry& entry = entries[it->second];
        assert(entry.state != ATTEntry::FREE && entry.phy_tag == phy_tag);
        // LRU
        queues[entry.state].remove(it->second);
        queues[entry.state].pushBack(it->second);
        return it->second;
    }
}

int
AddrTransTable::insert(Tag phy_tag, Addr hw_addr, ATTEntry::State state,
        Profiler& profiler)
{
    assert(tagIndex.count(phy_tag) == 0);
    assert(!queues[ATTEntry::FREE].empty());

    int i = queues[ATTEntry::FREE].popFront();
    queues[state].pushBack(i);
    entries[i].state = state;
    entries[i].phy_tag = phy_tag;
    entries[i].hw_addr = hw_addr;

    tagIndex[phy_tag] = i;
    profiler.addTableOp();
    return i;
}

void
AddrTransTable::shiftState(int index, ATTEntry::State new_state,
        Profiler& profiler)
{
    ATTEntry& entry = entries[index];
    assert(entry.state != new_state);
    if (new_state == ATTEntry::FREE) {
        tagIndex.erase(entry.phy_tag);
    }
    queues[entry.state].remove(index);
    queues[new_state].pushBack(index);
    entries[index].state = new_state;
    profiler.addTableOp();
}

void
AddrTransTable::reset(int index, Addr new_base, ATTEntry::State new_state,
        Profiler& profiler)
{
    entries[index].hw_addr = new_base;
    shiftState(index, new_state, profiler);
}

void
AddrTransTable::clearStats(Profiler& profiler)
{
    for (vector<ATTEntry>::iterator it = entries.begin(); it != entries.end();
            ++it) {
        it->epoch_reads = 0;
        it->epoch_writes = 0;
    }
    profiler.addTableOp(); // assumed in parallel
}
