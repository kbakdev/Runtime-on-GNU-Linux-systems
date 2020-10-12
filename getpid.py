import os

def getpid():
    for itemname in os.listdir('/proc'):
        path = "/proc/%s" % itemname
        if os.path.isdir(path) and os.path.exists(path + "/maps") == True:
            try:
                exedir = os.path.dirname(os.readlink(path + "/exe"))
                if exedir == os.path.dirname(os.path.realpath(__file__)):
                    return itemname
            except OSError:
                pass

if __name__ == '__main__':
    print(getpid())