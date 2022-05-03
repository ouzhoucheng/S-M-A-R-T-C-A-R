import os 
import cv2 as cv 
import numpy as np 
from tqdm import tqdm
# #动物水果版本：
# all_data = []
# all_label = []
# numpy_data = []
# numpy_label = []
# numpy_flag = 0
# n=0
# back_pic = cv.imread("purple.png",cv.IMREAD_COLOR)
# back_pic = cv.resize(back_pic,(327,327))
# print(back_pic.shape)
# for i in range(10):
#     path = './data_x/%d'%i  
#     for f in tqdm(os.listdir(path)):      #返回该目录下所有文件的文件名
#         extension = os.path.splitext(f)[-1]
#         if ( extension == '.png' or extension == ".jpg"):      #根据jpg和png灵活选择
#             try:
#                 img = cv.imread(os.path.join(path, f),cv.IMREAD_COLOR)
#                 img = cv.resize(img,(283,283))
#             except:
#                 print("数据出错")
#                 error = 1
#                 if error ==1:
#                     continue
#             img_all = back_pic
#             img_all[22:305,22:305] = img
#             # cv.imshow("img_all",img_all)
#             # cv.waitKey(50)
#             if(not os.path.exists("./data_my/%d"%i)):
#                 os.makedirs(".\\data_my\\%d"%i)
#             cv.imwrite("./data_my/%d"%i+"/new_%d.png"%n,img_all)
#             n = n+1
#数字版本：
# all_data = []
# all_label = []
# numpy_data = []
# numpy_label = []
# numpy_flag = 0
# n=0
# i=10
# back_pic = cv.imread("purple.png",cv.IMREAD_COLOR)
# back_pic = cv.resize(back_pic,(327,327))
# print(back_pic.shape)
# path = './data/numbers'  
# for f in tqdm(os.listdir(path)):      #返回该目录下所有文件的文件名
#     extension = os.path.splitext(f)[-1]
#     if ( extension == '.png' or extension == ".jpg"):      #根据jpg和png灵活选择
#         img = cv.imread(os.path.join(path, f),cv.IMREAD_COLOR)
#         img_all = back_pic
#         img_all[22:305,22:305] = img
#         cv.imshow("img_all",img_all)
#         cv.waitKey(50)
#         if(not os.path.exists("./data_my/%d"%i)):
#             os.makedirs(".\\data_my\\%d"%i)
#         cv.imwrite("./data_my/%d"%i+"/%d.png"%n,img_all)
#         n = n+1
#         i = i+1
#空白框
# all_data = []
# all_label = []
# numpy_data = []
# numpy_label = []
# numpy_flag = 0
# n=0
# back_pic = cv.imread("purple.png",cv.IMREAD_COLOR)
# back_pic = cv.resize(back_pic,(327,327))
# img_all = back_pic
# img_all[22:305,22:305] = (255,255,255)
# cv.imshow("img_all",img_all)
# cv.waitKey(50)
# cv.imwrite("kongbai.png",img_all)

