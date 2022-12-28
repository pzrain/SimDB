import os
import time

os.system("mkdir -p database/")
os.system("rm -r -f database/*")
os.system("mkdir -p ans")
os.system("rm -r -f ans/*")
for fileName in list(os.listdir('.')):
    if fileName.endswith('.sql') and fileName.startswith('test'):
        startTime = time.time()
        os.system("../bin/SimDB 2>/dev/null 1>ans/%s.out %s" % (fileName[:-4], fileName))
        print("%s elapased %.2lfs" % (fileName, time.time() - startTime))
os.system("rm -r database")