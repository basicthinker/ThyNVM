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

#ifndef __THYNVM_PROFILER_HH__
#define __THYNVM_PROFILER_HH__

#include <cassert>
#include <cstdint>

namespace thynvm {

class Profiler
{
  public:
    Profiler();
    Profiler(const Profiler& p);

    void setOpLatency(uint64_t latency);
    void setBlockTraffic(uint64_t bytes);
    void setPageTraffic(uint64_t bytes);

    void addLatency(uint64_t lat);
    void addTableOp(int num = 1);
    void addBufferOp(int num = 1);

    void addBlockIntraChannel(int num = 1);
    void addBlockInterChannel(int num = 1);
    void addPageIntraChannel(int num = 1);
    void addPageInterChannel(int num = 1);

    uint64_t sumLatency();
    uint64_t sumTraffic(bool excluding_intra = false);

    void setIgnoreLatency();
    void clearIgnoreLatency();

    static Profiler Null;
    static Profiler Overlap;

  private:
    uint64_t _op_latency;
    uint64_t _block_bytes;
    uint64_t _page_bytes;

    uint64_t num_table_ops;
    uint64_t num_buffer_ops;
    uint64_t latency;

    uint64_t bytes_intra_channel;
    uint64_t bytes_inter_channel;

    bool _ignore_latency;
};

inline
Profiler::Profiler()
        : _op_latency(0), _block_bytes(0), _page_bytes(0),
          num_table_ops(0), num_buffer_ops(0),
          latency(0), bytes_intra_channel(0), bytes_inter_channel(0)
{
    _ignore_latency = false;
}

inline
Profiler::Profiler(const Profiler& p)
        : Profiler()
{
    _op_latency = p._op_latency;
    _block_bytes = p._block_bytes;
    _page_bytes = p._page_bytes;
    _ignore_latency = p._ignore_latency;
}

inline void
Profiler::setOpLatency(uint64_t latency)
{
    _op_latency = latency;
}

inline void
Profiler::setBlockTraffic(uint64_t bytes)
{
    _block_bytes = bytes;
}

inline void
Profiler::setPageTraffic(uint64_t bytes)
{
    _page_bytes = bytes;
}

inline void
Profiler::setIgnoreLatency()
{
    assert(!_ignore_latency);
    _ignore_latency = true;
}

inline void
Profiler::clearIgnoreLatency()
{
    assert(_ignore_latency);
    _ignore_latency = false;
}

inline void
Profiler::addTableOp(int num)
{
    if (!_ignore_latency) {
        num_table_ops += num;
    }
}

inline void
Profiler::addBufferOp(int num)
{
    if (!_ignore_latency) {
        num_buffer_ops += num;
    }
}

inline void
Profiler::addLatency(uint64_t lat)
{
    if (!_ignore_latency) {
        latency += lat;
    }
}

inline void
Profiler::addBlockIntraChannel(int num)
{
    bytes_intra_channel += num * _block_bytes;
}

inline void
Profiler::addBlockInterChannel(int num)
{
    bytes_inter_channel += num * _block_bytes;
}

inline void
Profiler::addPageIntraChannel(int num)
{
    bytes_intra_channel += num * _page_bytes;
}

inline void
Profiler::addPageInterChannel(int num)
{
    bytes_inter_channel += num * _page_bytes;
}

inline uint64_t
Profiler::sumLatency()
{
    assert(_op_latency >= 0);
    assert(!_ignore_latency);
    return latency + _op_latency * (num_table_ops + num_buffer_ops);
}

inline uint64_t
Profiler::sumTraffic(bool excluding_intra)
{
    return (excluding_intra ? 0 : bytes_intra_channel) + bytes_inter_channel;
}

}  // namespace thynvm

#endif  // __THYNVM_PROFILER_HH__

