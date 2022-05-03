'''
dataset.py: 生成训练数据
训练数据仅由10张标准数字图片组成。
使用COCO开源数据集进行数据增强，即将标准数字图片随机贴图到COCO数据集中的图片中，组成一张训练图片。
如果一张COCO数据集的图片没有被贴图数字图片，则将其作为负样本。
同时增加HSV增强、射影变换增强、模糊增强。
训练数据一共分为11类，数字0-9和负样本（无任何数字的图片）
'''
import numpy as np
import random
import math
import cv2
import os

# HSV增强（参考：YOLOv5）###########################################################
def augment_hsv(img, hgain=0.02, sgain=0.6, vgain=0.6):
    r = np.random.uniform(-0.6, 0.5, 3) * [hgain, sgain, vgain] + 1     #随机采样（-1到1，3个样本）
    hue, sat, val = cv2.split(cv2.cvtColor(img, cv2.COLOR_BGR2HSV))#转HSV并进行通道拆分 ：
    dtype = img.dtype  # uint8

    x = np.arange(0, 256, dtype=np.int16)#0为起点，256为终点的排列，数据类型int16：
    lut_hue = ((x * r[0]) % 180).astype(dtype)    #转换成int16数据类型：
    lut_sat = np.clip(x * r[1], 0, 255).astype(dtype)    #小于0的变成0，大于255的变成255：
    lut_val = np.clip(x * r[2], 0, 255).astype(dtype)
    img_hsv = cv2.merge((cv2.LUT(hue, lut_hue), cv2.LUT(sat, lut_sat), cv2.LUT(val, lut_val))).astype(dtype)    #合并通道(lut_hue:一维查找表，数组下标对应图片里面的灰度值)：
    cv2.cvtColor(img_hsv, cv2.COLOR_HSV2BGR, dst=img)  # no return needed    #再转换到RGB通道：


# 射影变换增强（参考：YOLOv5）######################################################
def random_perspective(img, degrees=10, translate=.0, scale=(0., 0.), shear=0.1, perspective=0.001, border=(0, 0)):
    # torchvision.transforms.RandomAffine(degrees=(-10, 10), translate=(.1, .1), scale=(.9, 1.1), shear=(-10, 10))
    # targets = [cls, xyxy]

    height = img.shape[0] + border[0] * 2  # shape(h,w,c)
    width = img.shape[1] + border[1] * 2

    # Center
    C = np.eye(3)    #生成单位矩阵(对角矩阵)
    #第1行第3列
    C[0, 2] = -img.shape[1] / 2  # x translation (pixels)
    #第2行第3列
    C[1, 2] = -img.shape[0] / 2  # y translation (pixels)

    # Perspective(分别的，各自的)
    P = np.eye(3)
    #随机生成一个数
    P[2, 0] = random.uniform(-perspective, perspective)  # x perspective (about y)
    P[2, 1] = random.uniform(-perspective, perspective)  # y perspective (about x)

    # Rotation and Scale(旋转和放大)
    R = np.eye(3)
    a = random.uniform(-degrees, degrees)
    # a += random.choice([-180, -90, 0, 90])  # add 90deg rotations to small rotations
    s = random.uniform(1 - scale[0], 1 + scale[1])
    #cv2.getRotationMatrix2D：构造旋转矩阵
    R[:2] = cv2.getRotationMatrix2D(angle=a, center=(0, 0), scale=s)#前两行

    # Shear(切变)
    S = np.eye(3)
    S[0, 1] = math.tan(random.uniform(-shear, shear) * math.pi / 180)  # x shear (deg)
    S[1, 0] = math.tan(random.uniform(-shear, shear) * math.pi / 180)  # y shear (deg)

    # Translation
    T = np.eye(3)
    T[0, 2] = random.uniform(0.5 - translate, 0.5 + translate) * width  # x translation (pixels像素)生成随机数
    T[1, 2] = random.uniform(0.5 - translate, 0.5 + translate) * height  # y translation (pixels)

    # Combined rotation matrix
    M = T @ S @ R @ P @ C  # 操作顺序从左到右(@：矩阵乘法)
    if (border[0] != 0) or (border[1] != 0) or (M != np.eye(3)).any():  # image changed
        if perspective:
            img = cv2.warpPerspective(img, M, dsize=(width, height), borderValue=(114, 114, 114))
        else:  # affine
            img = cv2.warpAffine(img, M[:2], dsize=(width, height), borderValue=(114, 114, 114))
    return img


# 运动模糊增强###################################################################
def motion_blur(image, degree=8, angle=60):
    image = np.array(image, dtype=np.float32)
    # 这里生成任意角度的运动模糊kernel的矩阵， degree越大，模糊程度越高
    M = cv2.getRotationMatrix2D((degree / 2, degree / 2), angle, 1)
    motion_blur_kernel = np.diag(np.ones(degree))#生成一个单位矩阵
    motion_blur_kernel = cv2.warpAffine(motion_blur_kernel, M, (degree, degree))#切变换
    motion_blur_kernel = motion_blur_kernel / degree#剪切+模糊
    blurred = cv2.filter2D(image, -1, motion_blur_kernel)#卷积进行滤波
    # convert to uint8
    cv2.normalize(blurred, blurred, 0, 255, cv2.NORM_MINMAX)#全部值按比例限制在0到255以内
    blurred = np.array(blurred, dtype=np.uint8)
    return blurred

'''
class_num: 分类数量（取11）
pos_dir: 正样本所在目录。该目录下应该分别有0~(class_num-1)共计class_num-1个子文件夹。
neg_dir: 负样本（背景图）所在目录。建议使用COCO开源数据集。该目录下应该有若干张背景图片。
size: 增强后的图片分辨率（目前取160*64）。背景图会被直接resize成该分辨率，而正样本会成比例随机贴图到背景图上。
num: 数据集的大小（None则取负样本的数据）
'''
class MyDataset:
    #初始化自动执行方法
    def __init__(self, class_num, pos_dir, neg_dir, num_dir,size, num=None):#自动调用
        super(MyDataset, self).__init__()
        self.class_num = class_num
        #获取正样本图片的完整路径：
        # self.pos_fns = [[f"{pos_dir}/{i}/{fn}" for fn in os.listdir(f"{pos_dir}/{i}")] for i in range(class_num - 1)]
        self.pos_fns = [[f"{pos_dir}/{i}/{fn}" for fn in os.listdir(f"{pos_dir}/{i}")] for i in range(class_num)]
        #获取负样本图片的完整路径：
        self.neg_fns = []
        for dir in neg_dir:#neg_dir：COCO数据集
            self.neg_fns.extend([f"{dir}/{fn}" for fn in os.listdir(dir)])#将数据集中的图片路径添加到列表
        self.size = (size, size) if not isinstance(size, tuple) else size#检验图片大小参数是否符合要求
        self.num = len(self.neg_fns) if num is None else num#负数据集大小存入num参数中
        # #读取数字完整路径：
        # self.num_fns = []
        # self.num_fns.extend([f"{num_dir}/{fn}" for fn in os.listdir(num_dir)])#将数字图片路径添加到列表
        # print(self.num_fns)
        print(len(self.pos_fns))
    
    #返回负数据集大小
    def __len__(self):
        return self.num

    # 生成一个训练数据(一张图片)。idx不起作用，因为每次生成都是随机选取图片。
    def __getitem__(self, idx):
        # 随机选一个背景图
        neg_fn = random.sample(self.neg_fns, 1)[0]
        #读取并修改背景图片大小
        neg_img = cv2.imread(neg_fn)
        neg_img = cv2.resize(neg_img, self.size)
        # 随机选一个类别
        cls_num = random.randint(0, self.class_num)
        size_w = size_h = min(self.size)#图片变正方形
        # 未超出长度则准备向背景图上贴图(为了生成负样本)
        if cls_num < len(self.pos_fns):
            print(cls_num)
            # 随机选取对应类别中的一张图片（数字图片每种类别只有一张图片）
            pos_fn = random.sample(self.pos_fns[cls_num], 1)[0]
            pos_img = cv2.imread(pos_fn)
            # 随机选取这张正样本图片的大小(0.2~1倍背景图大小)
            size_w = size_h = random.randint(min(self.size) // 2.5, min(self.size) // 2)
            pos_img = cv2.resize(pos_img, (size_w, size_h))
            # 随机选取一个贴图位置
            h, w, c = pos_img.shape #正样本的宽度高度
            x1 = random.randint((w + 1) // 2, self.size[0] - (w + 1) // 2) - w // 2
            y1 = random.randint((h + 1) // 2, self.size[1] - (h + 1) // 2) - h // 2
            neg_img[y1:y1 + h, x1:x1 + w] = pos_img#直接替换矩阵值来实现贴图操作
        # elif(cls_num==20):
        #     # 随机选取对应类别中的一张图片（数字图片每种类别只有一张图片）
        #     num_fn = random.sample(self.num_fns, 1)[0]#选取出来的元素会被当做列表返回，所以要加[0]
        #     num_img = cv2.imread(num_fn)
        #     # 随机选取这张正样本图片的大小(0.2~1倍背景图大小)
        #     size_w = size_h = random.randint(min(self.size) // 4, min(self.size) // 2)
        #     num_img = cv2.resize(num_img, (size_w, size_h))
        #     # 随机选取一个贴图位置
        #     h, w, c = num_img.shape #正样本的宽度高度
        #     x1 = random.randint((w + 1) // 2, self.size[0] - (w + 1) // 2) - w // 2
        #     y1 = random.randint((h + 1) // 2, self.size[1] - (h + 1) // 2) - h // 2
        #     neg_img[y1:y1 + h, x1:x1 + w] = num_img#直接替换矩阵值来实现贴图操作
        elif(cls_num==21):
            cls_num=20
        #超出范围的就直接输出为了负样本
        # random hsv
        augment_hsv(neg_img)
        #影射变换
        neg_img = random_perspective(neg_img)

        # 运动模糊增强
        # if random.random() < 0.5:
        #     neg_img = motion_blur(neg_img, degree=random.randint(2, size_w // 4), angle=random.randint(-180, 180))

        # to float
        neg_img = neg_img.astype(np.float64)
        # random noise
        neg_img += np.random.randn(*neg_img.shape) * 5
        # clip to 0~255(范围归到0到255)
        neg_img = np.clip(neg_img, 0, 255)
        # to u8
        neg_img = neg_img.astype(np.uint8)

        # FOR DEBUG
        # import time
        # neg_img = neg_img.astype(np.uint8)
        # cv2.imwrite(f"im2show/{cls_num}-{time.time()}.jpg", neg_img)
        # cv2.waitKey(0)

        # BGR to RGB
        neg_img = neg_img[..., (2, 1, 0)]

        return neg_img, cls_num


# 一次性生成一批训练数据，用来加速训练
if __name__ == "__main__":
    from multiprocessing import Pool
    from tqdm import tqdm

    # 一次性生成数据的数量
    NUM = 100

    data = MyDataset(21, "data_new", ["train2017"], "data",(120, 120))
    imgs = np.zeros([NUM, 120, 120, 3], dtype=np.uint8)  #预先全部填充为0
    clss = np.zeros([NUM], dtype=np.int32)  #类别也预先填充为0

    # 多进程并行加速。调整24为实际CPU数量。
    with Pool(8) as pool:
        #enumerate:同时返回索引和对应的元素
        for i, (img, cls) in tqdm(enumerate(pool.map(data.__getitem__, range(NUM))), total=NUM):
            imgs[i] = img 
            clss[i] = cls

    np.save("xs", imgs)
    np.save("ys", clss)
