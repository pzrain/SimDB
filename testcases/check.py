import os
import time
import argparse

ARG = argparse.ArgumentParser()
ARG.add_argument("--test", type=str, default=None)
ARG = ARG.parse_args()

os.system("rm -r -f database")
os.system("rm -r -f ans")
os.system("mkdir -p database/")
os.system("mkdir -p ans")
if ARG.test is not None:
    fileName = ARG.test
    if not (fileName.endswith('.sql') and fileName.startswith('test')) or not fileName in list(os.listdir('.')):
        print("Wrong fileName!")
    else:
        startTime = time.time()
        os.system("../bin/SimDB 2>/dev/null 1>ans/%s.out %s" % (fileName[:-4], fileName))
        print("%s elapased %.2lfs" % (fileName, time.time() - startTime))
else:
    for fileName in list(os.listdir('.')):
        if fileName.endswith('.sql') and fileName.startswith('test'):
            startTime = time.time()
            os.system("../bin/SimDB 2>/dev/null 1>ans/%s.out %s" % (fileName[:-4], fileName))
            print("%s elapased %.2lfs" % (fileName, time.time() - startTime))
os.system("rm -r database")
