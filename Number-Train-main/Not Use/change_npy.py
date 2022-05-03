from tqdm import tqdm   #进度条
import os
import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":

    # xs = np.load("xs.npy")
    # ys = np.load("ys.npy")
    # x1 = xs[1]
    # x2 = xs[2]
    # x3 = xs[3]
    # xs[1] = x3
    # xs[2] = x1
    # xs[3] = x2
    # np.save("xfinal",xs)
    xs = np.load("xfinal.npy")
    print(xs.shape)