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

#include "version_buffer.hh"

using namespace std;
using namespace thynvm;

uint64_t
VersionBuffer::allocSlot(Profiler& profiler)
{
    assert(!index_sets[FREE].empty());
    int i = *index_sets[FREE].begin();
    index_sets[FREE].erase(index_sets[FREE].begin());
    index_sets[IN_USE].insert(i);
    profiler.addBufferOp();
    return at(i);
}

void
VersionBuffer::freeSlot(uint64_t hw_addr, State state, Profiler& profiler)
{
    int i = index(hw_addr);
    set<int>::iterator it = index_sets[state].find(i);
    assert(it != index_sets[state].end());
    index_sets[state].erase(it);
    index_sets[FREE].insert(i);
    profiler.addBufferOp();
}

void
VersionBuffer::backupSlot(uint64_t hw_addr, State state, Profiler& profiler)
{
    int i = index(hw_addr);
    assert(state == SHORT || state == LONG);
    assert(index_sets[state].count(i) == 0);
    set<int>::iterator it = index_sets[IN_USE].find(i);
    assert(it != index_sets[IN_USE].end());
    index_sets[IN_USE].erase(it);
    index_sets[state].insert(i);
    profiler.addBufferOp();
}

void
VersionBuffer::clearBackup(Profiler& profiler)
{
    for (set<int>::iterator it = index_sets[SHORT].begin();
            it != index_sets[SHORT].end(); ++it) {
        index_sets[FREE].insert(*it);
    }
    index_sets[SHORT].clear();
    profiler.addBufferOp(); // assumed in parallel

    for (set<int>::iterator it = index_sets[LONG].begin();
            it != index_sets[LONG].end(); ++it) {
        index_sets[SHORT].insert(*it);
    }
    index_sets[LONG].clear();
    profiler.addBufferOp(); // assumed in parallel

    assert(index_sets[IN_USE].size() + index_sets[FREE].size()
            + index_sets[SHORT].size() == (unsigned int )_length);
}
