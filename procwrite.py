#!/usr/bin/env python3

import os
import getpid

curpid = getpid.getpid()
if curpid == None:
    print("Determining the PID has failed.")
else:
    fd = open("/proc/%s/mem" % curpid, 'r+b')

    fd.seek(0x4005DC)
    fd.write(b'\xEB\xD6')