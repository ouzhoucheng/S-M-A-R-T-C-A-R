from tensorflow.keras.layers import *
from tensorflow.keras.models import *


# 空间注意力模块
# shape: 当前层的输入大小
def Attention2D(shape):
    _in = Input(shape=shape)
    #对_in做一个一维卷积
    x = Conv2D(1, 1, 1)(_in)
    x = Activation('sigmoid')(x)
    #_in和x相乘的层
    _out = Multiply()([_in, x])
    model = Model(inputs=(_in,), outputs=(_out,))
    return model


# 创建一个模型
# width：控制模型的宽度（衡量计算量和精度）实际上选取0.5
def create_model160x64(width=0.5):
    model = Sequential([
        InputLayer(input_shape=(120, 120, 3)),
        # 160x64 120*120
        Conv2D(int(16 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 80x32
        Conv2D(int(32 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((32, 80, int(32 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 40x16
        Conv2D(int(64 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((16, 40, int(64 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 20x8
        Conv2D(int(128 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((8, 20, int(128 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 10x4
        Conv2D(int(256 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((4, 10, int(256 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 5x2
        Conv2D(int(512 * width), 1, 1, padding='same'),
        Attention2D((3, 3, int(512 * width))),
        # MaxPooling2D((2, 5)),
        GlobalMaxPooling2D(),
        # 1x1
        # Flatten(),
        Dense(11), Softmax()
    ])
    return model
#GlobalMaxPooling2D：某种代替全连接层的方法
#将最后一层的特征图进行整张图的一个均值池化，形成一个特征点，将这些特征点组成最后的特征向量
#进行softmax中进行计算

# 导出tflite时使用的模型结构，主要是将GlobalMaxPooling2D替换成了MaxPooling2D和Flatten
# 不知道为什么，OpenArt-mini上使用GlobalMaxPooling2D会报错
# 而直接使用MaxPooling2D和Flatten进行训练迷之效果不好
def export_model(width=0.5):
    model = Sequential([
        InputLayer(input_shape=(120, 120, 3)),
        # 160x64
        Conv2D(int(16 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 80x32
        Conv2D(int(32 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((32, 80, int(32 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 40x16
        Conv2D(int(64 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((16, 40, int(64 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 20x8
        Conv2D(int(128 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((8, 20, int(128 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 10x4
        Conv2D(int(256 * width), 3, 1, padding='same'), BatchNormalization(), ReLU(6),
        Attention2D((4, 10, int(256 * width))),
        MaxPooling2D(),
        SpatialDropout2D(0.25),
        # 5x2
        Conv2D(int(512 * width), 1, 1, padding='same'),
        Attention2D((3, 3, int(512 * width))),
        MaxPooling2D((2, 2)),
        # GlobalMaxPooling2D(),
        # 1x1
        Flatten(),
        Dense(11), Softmax()
    ])
    return model
model = create_model160x64(width = 1)
print(model.summary())