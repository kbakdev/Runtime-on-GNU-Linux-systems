import os

mypid = os.readlink("/proc/self")
print("My PID: %s" % mypid)