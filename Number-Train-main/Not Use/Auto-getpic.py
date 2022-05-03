from numpy.lib import index_tricks
import pyautogui as gui
import time
import win32gui, win32ui, win32con, win32api
import cv2
import numpy as np
import os

def rect_follow():
    gui.moveTo(597,837,duration = 1)
    gui.moveRel(800,0,duration = 1)
    gui.moveRel(0,-800,duration = 1)
    gui.moveRel(-800,0,duration = 1)

#左上角：597,37 宽度：800


def grab_screen(region=None):
    """
    region:tuple,(left, top, right,down)
    """

    hwin = win32gui.GetDesktopWindow()
    if region:
        left, top, x2, y2 = region
        width = x2 - left
        height = y2 - top
    else:
        width = win32api.GetSystemMetrics(win32con.SM_CXVIRTUALSCREEN)
        height = win32api.GetSystemMetrics(win32con.SM_CYVIRTUALSCREEN)
        left = win32api.GetSystemMetrics(win32con.SM_XVIRTUALSCREEN)
        top = win32api.GetSystemMetrics(win32con.SM_YVIRTUALSCREEN)
        
    hwindc = win32gui.GetWindowDC(hwin)
    srcdc = win32ui.CreateDCFromHandle(hwindc)
    memdc = srcdc.CreateCompatibleDC()
    bmp = win32ui.CreateBitmap()
    bmp.CreateCompatibleBitmap(srcdc, width, height)
    memdc.SelectObject(bmp)
    memdc.BitBlt((0, 0), (width, height), srcdc, (left, top), win32con.SRCCOPY)
    signedIntsArray = bmp.GetBitmapBits(True)
    img = np.frombuffer(signedIntsArray, dtype='uint8')
    # print(img.shape)
    img.shape = (height, width, 4)
    # print(img.shape)
    srcdc.DeleteDC()
    memdc.DeleteDC()
    win32gui.ReleaseDC(hwin, hwindc)
    win32gui.DeleteObject(bmp.GetHandle())
    return img

i=0
index = 4
print(gui.position())
# gui.moveTo((591,33),duration=1)
# gui.moveTo((591,833),duration=1)
#比赛模式
for j in range(index):
    gui.click(976,667)
    time.sleep(0.5)
    gui.click(1239,1043)
    start = time.time()
    while(True):    # gui.click(1340,1039)
        gui.typewrite(" ")
        img = grab_screen((591,33,1391,833))
        img = cv2.resize(img,(120,120))
        if(not os.path.exists("./data_get/%d"%j)):
            os.makedirs(".\\data_get\\%d"%j)
        cv2.imwrite("./data_get/%d"%j+"/new_%d.png"%i,img)
        time.sleep(0.7)
        i=i+1
        end = time.time()
        if((end-start)>120):
            break
#         if(not os.path.exists("./data_my/%d"%i)):
#             os.makedirs(".\\data_my\\%d"%i)
#         cv.imwrite("./data_my/%d"%i+"/%d.png"%n,img_all)



#训练模式(鼠标)
# gui.click(895,1047)
# for j in range(2):
#     if(j==0):
#         gui.click(1123,1017)
#     if(j==1):
#         gui.click(1123,1065)
#     for i in range(index):
#         gui.click(gui.position())
#         img = grab_screen((597,37,1397,837))
#         img = cv2.resize(img,(120,120))
#         if(not os.path.exists("./data_get/%d"%j)):
#             os.makedirs(".\\data_get\\%d"%j)
#         cv2.imwrite("./data_get/%d"%j+"/new_%d.png"%i,img)
#         time.sleep(0.7)

# #训练模式(键盘)
#gui.click(895,1059)
# for j in range(2):
    # if(j==0):
    #     key = " "
    # if(j==1):
    #     key = " "
#     for i in range(index):
#         gui.typewrite(key)#更改相应键位
#         img = grab_screen((597,37,1397,837))
#         img = cv2.resize(img,(120,120))
#         if(not os.path.exists("./data_get/%d"%j)):
#             os.makedirs(".\\data_get\\%d"%j)
#         cv2.imwrite("./data_get/%d"%j+"/new_%d.png"%i,img)
#         time.sleep(0.6)
