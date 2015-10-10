#
#  HybridMemConfig.py
#
#  Created by Jinglei Ren on Oct 5, 2015.
#  Copyright (c) 2015 Jinglei Ren <jinglei.ren@persper.com>
#

from m5.objects import Addr, AddrRange, DRAMCtrl, VirtualXBar
from m5.util import addToPath

addToPath('../common')
import MemConfig

def config_hybrid_mem(options, system):
    """
    Assign proper address ranges for DRAM and NVM controllers.
    Create memory controllers and add their shared bus to the system.
    """
    system.thnvm_bus = VirtualXBar()
    mem_ctrls = []

    # The default behaviour is to interleave memory channels on 128
    # byte granularity, or cache line granularity if larger than 128
    # byte. This value is based on the locality seen across a large
    # range of workloads.
    intlv_size = max(128, system.cache_line_size.value)

    total_size = Addr(options.mem_size)
    dram_size = pow(2, options.page_bits) * options.ptt_length

    if dram_size < total_size.value:
        nvm_cls = MemConfig.get(options.nvm_type)
        nvm_range = AddrRange(0, total_size - dram_size - 1)
        nvm_ctrl = MemConfig.create_mem_ctrl(nvm_cls, nvm_range,
                                             0, 1, 0, intlv_size)
        # Set the number of ranks based on the command-line
        # options if it was explicitly set
        if issubclass(nvm_cls, DRAMCtrl) and options.mem_ranks:
            nvm_ctrl.ranks_per_channel = options.mem_ranks

        mem_ctrls.append(nvm_ctrl)

    if dram_size > 0:
        dram_cls = MemConfig.get(options.dram_type)
        dram_range = AddrRange(total_size - dram_size, total_size - 1)
        dram_ctrl = MemConfig.create_mem_ctrl(dram_cls, dram_range,
                                              0, 1, 0, intlv_size)
        # Set the number of ranks based on the command-line
        # options if it was explicitly set
        if issubclass(dram_cls, DRAMCtrl) and options.mem_ranks:
            dram_ctrl.ranks_per_channel = options.mem_ranks

        mem_ctrls.append(dram_ctrl)

    system.mem_ctrls = mem_ctrls

    # Connect the controllers to the THNVM bus
    for i in xrange(len(system.mem_ctrls)):
        system.mem_ctrls[i].port = system.thnvm_bus.master

    system.thnvm_bus.slave = system.membus.master

