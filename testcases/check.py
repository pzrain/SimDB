import os
import time
import argparse

ARG = argparse.ArgumentParser()
ARG.add_argument("--test", type=str, default=None)
ARG = ARG.parse_args()

def checkFile(outFile, ansFile):
    if not os.path.exists(ansFile):
        return -2
    with open(outFile) as f:
        outLines = f.readlines()
    with open(ansFile) as f:
        ansLines = f.readlines()
    if len(outLines) != len(ansLines):
        return 0
    for i in range(len(outLines)):
        outLine = outLines[i]
        ansLine = ansLines[i]
        if len(outLine) >= 8 and len(ansLine) > 8 and outLine[-8:-1] == "seconds" and ansLine[-8:-1] == "seconds":
            spaceCnt = 0
            indexOut = -1
            for j in range(len(outLine) - 1, -1, -1):
                if outLine[j] == " ":
                    spaceCnt += 1
                if spaceCnt == 2:
                    indexOut = j
                    break
            spaceCnt = 0
            for j in range(len(ansLine) - 1, -1, -1):
                if ansLine[j] == " ":
                    spaceCnt += 1
                if spaceCnt == 2:
                    indexAns = j
                    break
            outLine = outLine[:indexOut]
            ansLine = ansLine[:indexAns]
        if outLine != ansLine:
            return i
    return -1

os.system("rm -r -f database")
os.system("rm -r -f out")
os.system("mkdir -p database/")
os.system("mkdir -p out")
if ARG.test is not None:
    fileName = ARG.test
    if not (fileName.endswith('.sql') and fileName.startswith('test')) or not fileName in list(os.listdir('.')):
        print("Wrong fileName!")
    else:
        startTime = time.time()
        os.system("../bin/SimDB 2>/dev/null 1>out/%s.out %s" % (fileName[:-4], fileName))
        res = checkFile("out/%s.out" % fileName[:-4], "ans/%s.ans" % fileName[:-4])
        elapsedTime = time.time() - startTime
        if res == -1:
            print("Pass %s elapased %.2lfs" % (fileName, elapsedTime))
        elif res >= 0:
            print("Fail on Line %d, %s elapased %.2lfs" % (res + 1, fileName, elapsedTime))
        elif res == -2:
            print("No check on %s elapased %.2lfs" % (fileName, elapsedTime))
else:
    for fileName in list(os.listdir('.')):
        if fileName.endswith('.sql') and fileName.startswith('test'):
            startTime = time.time()
            os.system("../bin/SimDB 2>/dev/null 1>out/%s.out %s" % (fileName[:-4], fileName))
            res = checkFile("out/%s.out" % fileName[:-4], "ans/%s.ans" % fileName[:-4])
            elapsedTime = time.time() - startTime
            if res == -1:
                print("Pass %s elapased %.2lfs" % (fileName, elapsedTime))
            elif res >= 0:
                print("Fail on Line %d, %s elapased %.2lfs" % (res + 1, fileName, elapsedTime))
            elif res == -2:
                print("No check on %s elapased %.2lfs" % (fileName, elapsedTime))
os.system("rm -r database")
