import random
class Feistel(object):
    def __init__(self,  seed : int = 9961, \
                        encryptTime : int = 16, \
                        charSetSize : int = 4, \
                        function = lambda x, y: ((x + 1) ** y % ((x + 1) * y)) + x + y, \
                        debugging : bool = False \
                ) -> None:
        self.seed = seed
        self.encryptTime = encryptTime
        random.seed(self.seed)        
        self.subseed = self.__generate_sub_seed()
        self.function = function        # Feistel网络中的非线性函数
        self.charSetSize = charSetSize  # 几个字符为一组
        self.debug = debugging
        assert(charSetSize >= 4)        # 要求至少四个字符为一组（一个字符两个字节有点少，希望至少是八个字节）
    
    @property
    def __MOD(self):
        return 2 ** (self.charSetSize * 8 // 2)
    
    def __generate_sub_seed(self):
        return [random.randint(0, 0xffff_ffff_ffff_ffff_ffff_ffff_ffff_ffff) for _ in range(self.encryptTime)]
    
    def __Feistel_block(self, L0, R0, K1):
        L1 = R0
        R1 = L0 ^ ((self.function(R0, K1) % self.__MOD))  # 保证输出空间在[0,0xffffffff]之间
        return L1, R1
    
    def __Inv_Feistel_block(self, L1, R1, K1):
        R0 = L1
        L0 = R1 ^ ((self.function(L1, K1) % self.__MOD))
        return L0, R0
    
    def __setEncrypt(self, L, R):       # 分组后的加密
        for i in range(self.encryptTime):
            L, R = self.__Feistel_block(L, R, self.subseed[i])
        return L, R
    
    def __setDecrypt(self, L, R):       # 分组后的解密
        for i in range(self.encryptTime):
            L, R = self.__Inv_Feistel_block(L, R, self.subseed[self.encryptTime - 1 - i])
        return L, R
    
    def __setString2bit(self, s):       # 分组后的字符串转换成比特数
        ans = 0
        for ch in s:
            ans = (ans << 8) + ord(ch)
        return ans
    
    def __setBit2string(self, n):       # 分组后的比特转换成字符串
        lst = []
        for _ in range(self.charSetSize):
            ch = chr(n & 0xff)
            lst.append(ch)
            n = n >> 8
        lst = lst[::-1]
        return  "".join(lst)
    
    def encrypt(self, s):
        rem = len(s) % self.charSetSize
        if rem != 0:
            s = s + (self.charSetSize - rem) * " "               # padding
        lst = []
        for i in range(int(len(s)) // self.charSetSize):        # 计算字母所对应的ascii码并分组
            l = i * self.charSetSize
            substr = s[l: l + self.charSetSize]
            if self.debug == True:
                print("子串为: {:s}, 转换为ascii码后对应的十六进制数字: 0x{:x}".format(substr, self.__setString2bit(substr)))
            lst.append(self.__setString2bit(substr))
        ans = []
        bitLen = 8 * self.charSetSize                           # 对每组进行加密
        for bits in lst:
            modulo = 2 ** (bitLen // 2)
            R = bits % modulo
            L = (bits - R) >> (bitLen // 2)
            Ln, Rn = self.__setEncrypt(L, R)
            val = (Ln << (bitLen // 2)) + Rn
            ans.append(val)
            print("明文数字: 0x{:x}, 密文数字: 0x{:x}".format(bits, val))
        ret = 0
        for bits in ans:
            ret = (ret << bitLen) + bits
        return ret                              # 最后返回一个极大的数字，看成二进制即可
    
    def decrypt(self, bits: int, n: int):       # n为字符个数，bits为比特
        ans = []
        if n % self.charSetSize != 0:           # 填充
            n = n + (self.charSetSize - n % self.charSetSize)
        for i in range(n // self.charSetSize):  # 运用位运算从整数提取出分组后的比特信息
            val = bits & (2 ** (self.charSetSize * 8) - 1)
            ans.append(val)
            bits = bits >> (self.charSetSize * 8)
        ans = ans[ : :-1]                       # 由于ans最后面的bits存的是最开始的字串，故需要倒转
        s = ""
        bitLen = 8 * self.charSetSize           # 计算比特长度
        for idx, val in enumerate(ans):
            modulo = 2 ** (bitLen // 2)
            R = val % modulo
            L = (val - R) >> (bitLen // 2)      # 计算L, R
            L0, R0 = self.__setDecrypt(L, R)
            decryptVal = (L0 << (bitLen // 2)) + R0
            s += self.__setBit2string(decryptVal)
            print("密文: 0x{:x}；明文: 0x{:x}；子串: {:s}".format(val, decryptVal, self.__setBit2string(decryptVal)), hex(val ^ decryptVal))
        return s.strip(" ")                     # 去除空格
    
raw_txt = "CQUINFORMATIONSECURITYEXP" # cqu information security exp
def f(x, y):
    return (0x996177f3 ^ x) * (y & 0x3312ff78)
model = Feistel(seed = 9961, encryptTime = 16, charSetSize = 8, function = f, debugging = True)
val = model.encrypt(raw_txt)
s = model.decrypt(val, len(raw_txt))
print("最终解密结果：",s)