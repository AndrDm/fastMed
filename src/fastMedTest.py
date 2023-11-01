import os
import numpy as np
from random import random
from time import time
from ctypes import CDLL, POINTER, byref, c_size_t, c_double

mylib = CDLL(os.path.dirname(os.path.abspath(__file__)) + os.path.sep + "libMed.dll")
ND_POINTER_1 = np.ctypeslib.ndpointer(dtype=np.float64, ndim=1, flags="C")
mylib.MedianDBL.argtypes = [ND_POINTER_1, c_size_t, POINTER( c_double )]
mylib.MedianDBL.restype = np.int32
# Test Data
T1M = np.array([random() for _ in range(1024*1024+1)])
T4M = np.array([random() for _ in range(2048*2048+1)])
T16M = np.array([random() for _ in range(4096*4096+1)])
MyRes = c_double( 0.0 )

start = time()
for x in range(10):mylib.MedianDBL(T1M, T1M.size, byref(MyRes))
print ("Histo 1M:", (time() - start)*100, "ms; res = ", MyRes)
                  
start = time()          
for x in range(10):NumpyRes = np.median(T1M)
print ("Numpy 1M:", (time() - start)*100, "ms; res = ", NumpyRes)
if (MyRes == NumpyRes): print("passed")

start = time()
for x in range(10):mylib.MedianDBL(T4M, T4M.size, byref(MyRes))
print ("Histo 4M:", (time() - start)*100, "ms; res = ", MyRes)

start = time()          
for x in range(10):NumpyRes = np.median(T4M)
print ("Numpy 4M:", (time() - start)*100, "ms; res = ", NumpyRes)
if (MyRes == NumpyRes): print("passed")

start = time()
for x in range(10):mylib.MedianDBL(T16M, T16M.size, byref(MyRes))
print ("Histo 16M:", (time() - start)*100, "ms; res = ", MyRes)

start = time()          
for x in range(10):NumpyRes = np.median(T16M)
print ("Numpy 16M:", (time() - start)*100, "ms; res = ", NumpyRes)
if (MyRes == NumpyRes): print("passed")
