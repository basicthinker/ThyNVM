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

#include "index_queue.hh"

using namespace std;

void
IndexQueue::remove(int i)
{
    assert(i >= 0);
    const int prev = array[i].prev;
    const int next = array[i].next;

    if (prev == -EINVAL) {
        assert(front() == i);
        setFront(next);
    } else {
        array[prev].next = next;
    }

    if (next == -EINVAL) {
        assert(back() == i);
        setBack(prev);
    } else {
        array[next].prev = prev;
    }

    array[i].prev = -EINVAL;
    array[i].next = -EINVAL;

    --_length;
}

void
IndexQueue::pushBack(int i)
{
    assert(i >= 0);
    assert(array[i].prev == -EINVAL && array[i].next == -EINVAL);
    if (empty()) {
        setFront(i);
        setBack(i);
    } else {
        array[i].prev = back();
        array[back()].next = i;
        setBack(i);
    }

    ++_length;
}

int
IndexQueue::accept(QueueVisitor* visitor)
{
    int num = 0, tmp;
    for (int i = front(); i != -EINVAL; ++num) {
        tmp = array[i].next;
        visitor->Visit(i);
        i = tmp;
    }
    return num;
}

