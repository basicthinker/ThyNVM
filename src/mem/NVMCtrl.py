#
#  NVMCtrl.py
#
#  Created by Jinglei Ren on Oct 5, 2014.
#  Copyright (c) 2015 Jinglei Ren <jinglei.ren@persper.com>
#

import DRAMCtrl

# A single DDR3-1600 x64 channel (one command and address bus), with
# timings based on a DDR3-1600 4 Gbit datasheet (Micron MT41J512M8) in
# an 8x8 configuration. Extended to simulate phase-change memory (PCM),
# according to:
#   Benjamin C. Lee, Engin Ipek, Onur Mutlu, and Doug Burger.
#   Architecting phase change memory as a scalable dram alternative.
#   In ISCA, 2009.
class DDR3_1600_x64_PCM(DRAMCtrl.DDR3_1600_x64):
    tRCD = '60ns'
    tRP = '150ns'

