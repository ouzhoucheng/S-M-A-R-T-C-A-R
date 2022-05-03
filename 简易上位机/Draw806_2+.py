import serial
import os
import matplotlib.pyplot as plt
import numpy as np
import imageio
import cv2 as cv

ser=serial.Serial("com7",460800,timeout=0.5) # winsows系统使用com7口连接串行口
print(ser.name) # 打印设备名称
# ser.open() #打开端口,调用时已经打开
# ImgLine = [0, 0, 0, 0] # 存储行的列表
img = np.ones((121, 220, 3), np.uint8) # 生成一个空RGB图像
gif_images = [] # gif图数组
img_count = 0

# 定义一些画图颜色
black = (0, 0, 0)
red = (0, 0, 255)
green = (0, 255, 0)
color1 = (127, 127, 0)
color2 = (0, 127, 127)
color3 = (127, 0, 127)
color4 = (255, 255, 0)
color5 = (0, 255, 255)
color6 = (255, 0, 255)
white = (255,255,255)
thickness = 1
lineType = 8
cnt = 0

# 在img画布上用color颜色画点(x,y)
def draw_cross(img,x,y,color):
    cv.line(img, (x-1, y), (x+1,y), color, thickness, lineType)
    cv.line(img, (x,y-1), (x,y+1), color, thickness, lineType)
# 写一些标志位注释
def write(text,line):#一共可以写12行
    org = (165,10*line)
    fontFace = cv.FONT_HERSHEY_COMPLEX
    fontScale = 0.5
    thickness = 1
    lineType = 4
    bottomLeftOrigin = False
    color = white
    # bottomLeftOrigin = True
    cv.putText(img, text, org, fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin)

# 没有数据发来时,等待,有则break
while (1):
    data = ser.readline()
    # print(data)
    MsgLine = list(data)
    if len(MsgLine) != 0:
        break
    print(len(MsgLine), "waiting for data, Ctrl+c to break")


cv.namedWindow("edge") # 命名窗口
# while循环画图
while (1):
    data = ser.readline() # 从串口读取数据
    ImgLine = list(data)
    if len(ImgLine) == 6 and ImgLine[0]<120: # 行数一定是小于120
        draw_cross(img,ImgLine[1],119-ImgLine[0],red)
        draw_cross(img,ImgLine[2],119-ImgLine[0],green)
        cnt += 1
    elif len(ImgLine) == 6 and ImgLine[0] == 155: # 画圆环相关标志线
        cv.line(img, (0, 119-ImgLine[2]), (159, 119-ImgLine[2]), color1, thickness, lineType)
        cv.line(img, (0, 119-ImgLine[3]), (159, 119-ImgLine[3]), color2, thickness, lineType)
        cv.line(img, (0, 119-ImgLine[4]), (159, 119-ImgLine[4]), color3, thickness, lineType)
        write("O: %d"%ImgLine[1],1)
    elif len(ImgLine) == 6 and ImgLine[0] == 165: # 画三岔相关标志线
        cv.line(img, (0, 119-ImgLine[2]), (159, 119-ImgLine[2]), color4, thickness, lineType)
        cv.line(img, (0, 119-ImgLine[3]), (159, 119-ImgLine[3]), color5, thickness, lineType)
        cv.line(img, (ImgLine[4], 0), (ImgLine[4], 119), color6, thickness, lineType)
        write("Y: %d"%ImgLine[1],2)
    if cnt>100:
        cnt = 0
        cv.imshow('edge', img)
        cv.imwrite("./Img_save/%d.jpg"%img_count, img)
        img = np.ones((121, 220, 3), np.uint8)  # 生成一个空RGB图像
        img_count += 1
        # gif_images.append(img)
        # cv.waitKey(1)
        if(cv.waitKey(1)==27):
            path = "./Img_save"
            pic_lst = os.listdir(path)
            for name in range(img_count):
                filename = os.path.join(path, str(name)+".jpg")
                gif_images.append(imageio.imread(filename))
            print(np.shape(gif_images))
            imageio.mimsave("814left.gif", gif_images, 'GIF', fps=20)
            break