from tensorflow.keras.layers import *
from tensorflow.keras.models import *


# 空间注意力模块
# shape: 当前层的输入大小
def Attention2D(inputs):
    #对_in做一个一维卷积
    x = Conv2D(1, 1, 1)(inputs)
    x = Activation('sigmoid')(x)
    #_in和x相乘的层
    x = Multiply()([inputs, x])
    return x

def conv2d_unit(x, filters, kernels, strides=1):    #单纯的卷积
    x = Conv2D(filters, kernels,
            padding='same',
            strides=strides)(x) #本身的激活函数为线性
    x = BatchNormalization()(x)
    x = ReLU(6)(x)
    return x

def residual_block(inputs, filters):    #残差模块
    x = conv2d_unit(inputs, filters, (1, 1))
    x = conv2d_unit(x, 2 * filters, (3, 3)) #二倍关系
    x = add([inputs, x])    #合并
    x = Activation('linear')(x) #线性激活函数(并不做任何处理)
    return x


def stack_residual_block(inputs, filters, n):   #堆栈模块
    """Stacked residual Block
    """
    x = residual_block(inputs, filters) #残差模块

    for i in range(n - 1):
        x = residual_block(x, filters)  #堆起多层残差

    return x


# 创建一个模型
# width：控制模型的宽度（衡量计算量和精度）实际上选取0.5
def create_model160x64(width):
    inputs = Input(shape=(120, 120, 3))
    # 160x64 120*120
    #Conv2D(int(16 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
    x = conv2d_unit(inputs,16*width,(3,3))
    x = MaxPooling2D()(x)
    x = SpatialDropout2D(0.25)(x)
    # 80x32
    # Conv2D(int(32 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
    x = conv2d_unit(x,32*width,(3,3))
    x = Attention2D(x)
    x = MaxPooling2D()(x)
    x = SpatialDropout2D(0.25)(x)

    x = conv2d_unit(x,64*width,(3,3))
    x = Attention2D(x)
    x = MaxPooling2D()(x)
    x = SpatialDropout2D(0.25)(x)
    x = stack_residual_block(x, 32, n=2)   

    # 20x8
    x = conv2d_unit(x,128*width,(3,3))
    x = Attention2D(x)
    x = MaxPooling2D()(x)
    x = SpatialDropout2D(0.25)(x)
    x = stack_residual_block(x, 64, n=4)

    # 10x4
    x = conv2d_unit(x,256*width,(3,3))
    x = Attention2D(x)
    x = MaxPooling2D()(x)
    x = SpatialDropout2D(0.25)(x)

    # 5x2
    x = Conv2D(int(512 * width), 1, 1, padding='same')(x)
    x = Attention2D(x)
    # MaxPooling2D((2, 5)),
    x = GlobalMaxPooling2D()(x)
    # 1x1
    # Flatten(),
    x = Dense(21, activation='softmax')(x)    #全连接层

    model = Model(inputs, x)    #最终构成模型
    return model
model = create_model160x64(1)
print(model.summary())
