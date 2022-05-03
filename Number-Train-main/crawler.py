# import re
# import uuid
# import requests
# import os
# import numpy
# import imghdr
# from PIL import Image

# #requests带上自己浏览器信息的请求头，默认允许重定向
# header=\
# {
#      'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36',
#      "referer":"https://image.baidu.com"
#     }

# # 获取百度图片下载图片
# def download_image(key_word, save_name, download_max):
#     download_sum = 0
#     str_gsm = '80'
#     # 把每个类别的图片存放在单独一个文件夹中
#     save_path = 'data_my' + '/' + save_name
#     if not os.path.exists(save_path):
#         os.makedirs(save_path)
#     while download_sum < download_max:
#         # 下载次数超过指定值就停止下载
#         if download_sum >= download_max:
#             break
#         str_pn = str(download_sum)
#         # 定义百度图片的路径
#         url = 'http://image.baidu.com/search/flip?tn=baiduimage&ie=utf-8&' \
#               'word=' + key_word + '&pn=' + str_pn + '&gsm=' + str_gsm + '&ct=&ic=0&lm=-1&width=0&height=0'
#         print('正在下载 %s 的第 %d 张图片.....' % (key_word, download_sum))
#         try:
#             # 获取当前页面的源码
#             result = requests.get(url, timeout=30,headers=header).text
#             # 获取当前页面的图片URL
#             img_urls = re.findall('"objURL":"(.*?)",', result, re.S)
#             if len(img_urls) < 1:
#                 break
#             # 把这些图片URL一个个下载
#             for img_url in img_urls:
#                 # 获取图片内容
#                 img = requests.get(img_url, timeout=30,headers=header)
#                 img_name = save_path + '/' + str(uuid.uuid1()) + '.jpg'
#                 # 保存图片
#                 with open(img_name, 'wb') as f:
#                     f.write(img.content)
#                 download_sum += 1
#                 if download_sum >= download_max:
#                     break
#         except Exception as e:
#             print('【错误】当前图片无法下载，%s' % e)
#             download_sum += 1
#             continue
#     print('下载完成')

#     # 删除不是JPEG或者PNG格式的图片


# def delete_error_image(father_path):
#     # 获取父级目录的所有文件以及文件夹
#     try:
#         image_dirs = os.listdir(father_path)
#         for image_dir in image_dirs:
#             image_dir = os.path.join(father_path, image_dir)
#             # 如果是文件夹就继续获取文件夹中的图片
#             if os.path.isdir(image_dir):
#                 images = os.listdir(image_dir)
#                 for image in images:
#                     image = os.path.join(image_dir, image)
#                     try:
#                         # 获取图片的类型
#                         image_type = imghdr.what(image)
#                         # 如果图片格式不是JPEG同时也不是PNG就删除图片
#                         if image_type is not 'jpeg' and image_type is not 'png':
#                             os.remove(image)
#                             print('已删除：%s' % image)
#                             continue
#                         # 删除灰度图
#                         img = numpy.array(Image.open(image))
#                         if len(img.shape) is 2:
#                             os.remove(image)
#                             print('已删除：%s' % image)
#                     except:
#                         os.remove(image)
#                         print('已删除：%s' % image)
#     except:
#         pass


# if __name__ == '__main__':
#     # 定义要下载的图片key和value，ps：key为搜索类别，value搜索到这个类别的图片存储文件夹的名字
#     # key_words = {'橙子':'0','狗':'1','榴莲':'2','马':'3','猫':'4','牛':'5','苹果':'6','葡萄':'7','香蕉':'8','猪':'9'}
#     key_words = {'橙子':'0','狗':'1'}
#     # 每个类别下载两千千个
#     max_sum = 200
#     for key_word in key_words:
#         save_name = key_words[key_word]
#         download_image(key_word, save_name, max_sum)

#     # 删除错误图片
#     delete_error_image('data_my/')
import time
import pyautogui as gui

print(gui.position())