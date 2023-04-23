import PIL
from PIL import Image
import numpy as np
import random

# config
srcPath = "Lena.bmp"
tarPath = "Lena_hiddenMsg.bmp"
msg = "CQUWATERMASKEXP"

# function
def readBmp(path: str):         # 1D - height, 2D - width
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

def saveBmp(arr, path: str):
    img = Image.fromarray(arr)
    img.save(path)    

def convertIdxToWH(val, h, w):
    return val // w, val % w

def LSB_encrypt(arr, msg, h, w):
    '''
    用于LSB加密和解密
    '''
    count = 0
    for ch in msg:
        val = ord(ch)
        for _ in range(8):
            b = val & 1
            val >>= 1
            x, y = convertIdxToWH(count, h, w)
            count += 1
            arr[x][y] = ((arr[x][y] // 2) * 2) + b
    return arr

def LSB_decrypt(arr, count, h, w):
    '''
    用于LSB加密和解密
    '''
    index = 0
    ans = ""
    val = 0
    while index < count:
        x, y = convertIdxToWH(index, h, w)
        val = (val << 1) + (int(arr[x][y]) & 1)
        index += 1
        if index % 8 == 0:
            true_val = 0
            for _ in range(8):
                true_val = (true_val << 1) + (val & 1)
                val >>= 1
            ans += chr(int(true_val))
            val = 0
    return ans
            
if __name__ == "__main__":
    imgarr, width, height = readBmp(srcPath)
    msgLen = len(msg) << 3
    imgarr_crypt = LSB_encrypt(imgarr, msg, height, width)
    saveBmp(imgarr_crypt, tarPath)
    # 读取加密后的文件
    imgarr_crypt_read, width, height = readBmp(tarPath)
    decrypt_msg = LSB_decrypt(imgarr_crypt_read, msgLen, height, width)
    print(decrypt_msg)
    