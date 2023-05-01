import PIL
from PIL import Image
import numpy as np
import random
import math

# ----config----
srcPath = "Lena.bmp"
tarPath = "Lena_hiddenMsg.bmp"
msg = "CQUWATERMASKEXP"
    # 取p = 7, q = 17, 则n = 119, phi(n) = 96, public_k = e = 5, private_k = d = 77
RSA_encrpyt_K = 5           # RSA算法密钥（加密签名用）
RSA_decrpyt_K = 77          # RSA算法公钥（解密验证用）
RSA_n = 119                 # RSA算法的n
LSB_K = 0.20204205          # LSB算法的密钥


# function
def convertIdxToWH(val, h, w):
    '''
    用于将一维的下标转换为二维的下标，返回h_idx和w_idx，其中h是图像高度，w是图像宽度
    '''
    return val // w, val % w

# RSA
class RSA(object):
    def __init__(self) -> None:
        pass
    def encrypt(encryptKey, s: str, n: int):
        '''
        每8位进行加密
        (解释一下为什么用字符串：字符串的每个字符都是一个字节，不需要像Python的大整数一样进行位运算，比较方便，同时Python的Byte感觉不是很好用)
        '''
        ans = ""
        for c in s:
            val = ord(c)
            val = (val ** encryptKey) % n
            assert(0x00 <= val <= 0xff)
            ans += chr(val)
        print("RSA加密（签名）结束，结果为:{:s}".format(ans))
        return ans
    def decrypt(decryptKey, s: str, n: int, isPrint = False):
        '''
        每8位进行解密
        '''
        ans = ""
        for c in s:
            val = ord(c)
            val = (val ** decryptKey) % n
            assert(0 <= val < 128)
            ans += chr(val)
        if isPrint:
            print("RSA解密结束，结果为:{:s}".format(ans))
        return ans

# BMPImageProcessor
class BmpImageProcessor():
    '''
    BMP图像处理
    '''
    def __init__(self) -> None:
        pass
    @staticmethod
    def readBmp(path: str):         # 1D - height, 2D - width
        '''
        用于读取BMP文件
        '''
        listImage = []
        with Image.open(path) as img:
            width, height = img.size
            pixels = img.load()
            for y in range(height):
                lst = []
                for x in range(width):
                    lst.append(pixels[x, y])
                listImage.append(lst)
        return np.array(listImage, dtype="uint8"), width, height
    @staticmethod
    def saveBmp(arr: np.array, path: str):
        '''
        用于保存BMP文件
        '''
        img = Image.fromarray(arr)
        img.save(path)    

class LSB(object):
    '''
    图像水印嵌入算法实现
    '''
    def __init__(self) -> None:
        pass
    
    def getEmbeddingArray(K: float, n: int, m: int):
        '''
        用于获取图片嵌入的下标
        '''        
        ans = []
        for _ in range(n):
            K = 1 - 2 * K ** 2
            loc = int(math.floor(((K + 1) / 2) * float(m)))
            while loc in ans:           # 如果要写的位置有重复，则直接进行下一轮迭代
                K = 1 - 2 * K ** 2
                loc = int(math.floor(((K + 1) / 2) * float(m)))
            ans.append(loc)
        return ans
    
    @staticmethod
    def encrypt(K: float, msg: str, h: int, w: int, img: np.array):
        '''
        function
        --------
        LSB算法加密
        
        params
        ------
        K: 密钥\\
        msg: 需要加密的信息\\
        h, w: 图像的高度，宽度\\
        img: 灰度图像
        
        return
        ------
        img: 加密后的灰度图像
        '''
        n = len(msg) << 3
        arr = LSB.getEmbeddingArray(K, n, h * w)
        for chIdx, ch in enumerate(msg):            # 对每个字节进行加密
            val = ord(ch)
            embedList = arr[8 * chIdx: 8 * (chIdx + 1)]     # 获取当前字节的八个比特在图片中的隐写位置
            for i in range(8):
                b = (val >> (7 - i)) & 1                    # 获取比特位，从高（指数位权值高）到低写
                embedIdx = embedList[i]
                h_idx, w_idx = convertIdxToWH(embedIdx, h, w)
                img[h_idx][w_idx] = (img[h_idx][w_idx] & 0xfe) + b      # 抹除最低位，再填上最低位
        return img
        
    @staticmethod
    def decrypt(K: float, h: int, w: int, img: np.array):
        '''
        function
        --------
        LSB算法解密
        
        params
        ------
        K: 密钥\\
        h, w: 图像的高度，宽度\\
        img: 灰度图像
        
        return
        ------
        msg: 解密后的信息
        '''
        arr = LSB.getEmbeddingArray(K, 32, h * w)
        length = 0
        # 获取长度信息
        for i in range(4):
            embedList = arr[8 * i: 8 * (i + 1)]
            val = 0
            for j in range(8):
                h_idx, w_idx = convertIdxToWH(embedList[j], h, w)
                b = img[h_idx][w_idx] & 1
                val = (val << 1) + b
            # 这里注意要先解密才能正常解析
            decrypt_val = ord(RSA.decrypt(RSA_decrpyt_K, chr(val), RSA_n))
            length = (length << 8) + decrypt_val
        print("LSB解密阶段，获取到的密文长度为:{:d}".format(length))
        arr = LSB.getEmbeddingArray(K, 32 + length * 8, h * w)
        arr = arr[32: ]
        ans = ""
        # 开始获取密文
        for i in range(length):
            embedList = arr[8 * i: 8 * (i + 1)]
            val = 0
            # 对每个字节（字符）进行提取
            for j in range(8):
                h_idx, w_idx = convertIdxToWH(embedList[j], h, w)
                b = img[h_idx][w_idx] & 1
                val = (val << 1) + b
            ans += chr(val)
        print("LSB解密结束，得到的密文是{:s}（已去除头部长度信息）".format(ans))
        return ans
            
if __name__ == "__main__":
    # =========加密部分=========
        # 给信息加上头部，存储信息量
    val = len(msg)
    msg = chr((val & 0xff000000) >> 24) + chr((val & 0xff0000) >> 16) + chr((val & 0xff00) >> 8) + chr(val & 0xff) + msg
    print("原文的长度为{:d}，处理后的原文长度为{:d}，其中增加的4的来源是4字节的长度位，标识隐藏信息量".format(val, len(msg)))
        # 将信息用RSA算法进行数字签名
    signed_msg = RSA.encrypt(RSA_encrpyt_K, msg, RSA_n)
        # 使用LSB算法进行嵌入
    img, width, height = BmpImageProcessor.readBmp(srcPath)      # 读取图片
    img_encrypt = LSB.encrypt(K = LSB_K, msg = signed_msg, h = height, w = width, img = img)
    BmpImageProcessor.saveBmp(img_encrypt, tarPath)
    
    # =========解密部分=========
    
        # 读取加密后的文件
    img_encrypted_read, width, height = BmpImageProcessor.readBmp(tarPath)
        # 将隐藏信息提取出来
    decrypt_msg_LSB = LSB.decrypt(K = LSB_K, h = height, w = width, img = img_encrypted_read)
        # 对密文进行解密
    decrypt_msg_RSA = RSA.decrypt(RSA_decrpyt_K, decrypt_msg_LSB, RSA_n, isPrint=True)
    
    print("每个比特的隐写位置(一维)为：")
    print(LSB.getEmbeddingArray(LSB_K, len(msg) * 8, width * height))