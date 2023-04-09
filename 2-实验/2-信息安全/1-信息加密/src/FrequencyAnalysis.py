encrypted_txt = "UZ QSO VUOHXMOPV GPOZPEVSG ZWSZ OPFPESX UDBMETSX AIZ VUEPHZ HMDZSHZO WSFP APPD TSVP QUZW YMXUZUHSX EPYEPOPDZSZUFPO MB ZWP FUPZ HMDJ UD TMOHMQ"
freq_list = ["E", \
             "T", "A", "O", "I", "N", "S", "H", "R", \
             "D", "L", \
             "C", "U", "M", "W", "F", "G", "Y", "P", "B", \
             "V", "K", "J", "X", "Q", "Z"]
def freq_statistic(txt):
    lst = [0 for _ in range(26)]
    tot = 0
    for ch in txt:
        if ch == ' ':
            continue
        else:
            tot += 1
            idx = ord(ch) - ord('A')
            lst[idx] += 1
    for idx, val in enumerate(lst):
        lst[idx] = (chr(idx + ord('A')), val / tot)
    return lst

def getMapping(freq_list, statistic_list):
    statistic_list.sort(key = lambda x: x[1], reverse=True)
    decrypt_dic, encrypt_dic = {}, {}
    for i in range(26):
        decrypt_dic[statistic_list[i][0]] = freq_list[i]
        encrypt_dic[freq_list[i]] = statistic_list[i][0]
    decrypt_dic[" "] = " "
    encrypt_dic[" "] = " "
    return decrypt_dic, encrypt_dic
        
def decrypt(mapping, txt):
    ans = ""
    for ch in txt:
        ans+= mapping[ch]
    return ans
    
def mappingAdjusting(decrypt_mapping, encrypt_mapping, v1, v2):
    k1 = "A"
    k2 = "A"
    for k, v in decrypt_mapping.items():
        if (v == v1):
            k1 = k
        if (v == v2):
            k2 = k
    decrypt_Mapping[k1] = v2
    decrypt_Mapping[k2] = v1
    encrypt_Mapping[v1] = k2
    encrypt_Mapping[v2] = k1
    return decrypt_mapping, encrypt_mapping

def encoderPrint(decrypt_mapping):
    for k, v in decrypt_mapping.items():
        if (k == " "): 
            continue
        print("{:s} -> {:s}".format(k, v))

def interactive_decrypter(encrypted_txt, decrypt_Mapping, encrypt_Mapping):
    count = 1
    while True:
        decrypt_txt = decrypt(decrypt_Mapping, encrypted_txt)
        print("第 {:d} 次的解密结果为: {:s}\n\t请输入您认为应该变换的字母(输入格式：I P)，若认为是最终结果，请输入-1：".format(count, decrypt_txt))
        a = input()
        if a.strip(" ") == "-1":
            print("破译结束。")
            print("密码本为：")
            encoderPrint(decrypt_Mapping)
            print("破译结果为：")
            print(decrypt_txt)
            break
        else:
            v1, v2 = a.split()
        count += 1
        decrypt_Mapping, encrypt_Mapping = mappingAdjusting(decrypt_Mapping, encrypt_Mapping, v1, v2)

lst = freq_statistic(encrypted_txt)
decrypt_Mapping, encrypt_Mapping = getMapping(freq_list, lst)
decrypt_txt = decrypt(decrypt_Mapping, encrypted_txt)
interactive_decrypter(encrypted_txt, decrypt_Mapping, encrypt_Mapping)