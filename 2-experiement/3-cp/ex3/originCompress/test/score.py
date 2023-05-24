import os, platform, subprocess, shutil, sys

def score_compiler(arg1):
    record = {}

    is_windows = platform.system() == "Windows"

    assert(len(sys.argv) == 2)

    oftype = ""
    step = "-" + arg1
    if step == "-s0":
        oftype = "tk"
    elif step == "-s1":
        oftype = "json"
    elif step == "-s2":
        oftype = "out"
    elif step == "-S":
        oftype = "out"
        
    else:
        print("illegal input")
        exit()

    output_base = "./output/"
    testcase_base = "./testcase/"
    ref_base = "./ref/" + arg1 + "/"

    score = 0
    total = 58      # FIXME 改成读取测试用例数而不是写死

    if step == "-s0":
        for i in ["basic", "function"]:
            output_dir = output_base + i + '/'
            ref_dir = ref_base + i + '/'
            if os.path.exists(output_dir):
                files = os.listdir(output_dir)
                for file in files:
                    if not (file[-3:] == ".tk"):
                        continue
                    cmd = ' '.join(["diff", ref_dir + file, output_dir + file, '-w'])
                    if is_windows:
                        cmd = cmd.replace('/','\\')
                    # print(cmd)
                    cp = subprocess.run(cmd, shell=True, stderr=subprocess.DEVNULL, stdout=subprocess.PIPE)
                    if cp.returncode != 0:
                        record[file] = {"retval": cp.returncode, "err_detail": "diff test failed"}
                    else:
                        score += 1
                        record[file] = {"retval": 0}
                    print(file, record[file])
        print("score:",score,"/",total)
    elif step == "-s1":
        for i in ["basic", "function"]:
            output_dir = output_base + i + '/'
            ref_dir = ref_base + i + '/'
            if os.path.exists(output_dir):
                files = os.listdir(output_dir)
                for file in files:
                    if not (file[-5:] == ".json"):
                        continue
                    cmd = ' '.join(["diff", ref_dir + file, output_dir + file, '-w'])
                    if is_windows:
                        cmd = cmd.replace('/','\\')
                    cp = subprocess.run(cmd, shell=True, stderr=subprocess.DEVNULL, stdout=subprocess.PIPE)
                    if cp.returncode != 0:
                        record[file] = {"retval": cp.returncode, "err_detail": cp.stdout}
                    else:
                        score += 1
                        record[file] = {"retval": 0}
                    print(file, record[file])
        print("score:",score,"/",total)
    elif step == "-s2":
        for i in ["basic", "function"]:
            output_dir = output_base + i + '/'
            ref_dir = ref_base + i + '/'
            if os.path.exists(output_dir):
                files = os.listdir(output_dir)
                for file in files:
                    if not (file[-4:] == ".out"):
                        continue
                    cmd = ' '.join(["diff", ref_dir + file, output_dir + file, '-wB'])
                    if is_windows:
                        cmd = cmd.replace('/','\\')
                    # print(cmd)
                    cp = subprocess.run(cmd, shell=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL)
                    if cp.returncode != 0:
                        record[file] = {"retval": cp.returncode, "err_detail": cp.stderr}
                    else:
                        score += 1
                        record[file] = {"retval": 0}
                    print(file, record[file])
        print("score:",score,"/",total)
    elif step == "-S":
        for i in ["basic", "function"]:
            testcase_dir = testcase_base + i + '/'
            output_dir = output_base + i + '/'
            ref_dir = "./ref/s2/" + i + '/'
            if os.path.exists(output_dir):
                files = os.listdir(output_dir)
                for file in files:
                    if not (file[-2:] == ".s"):
                        continue

                    # gcc
                    fname, ftype = file.split('.')
                    ref_file = ref_dir + fname + ".out"
                    output_file = output_dir + fname + ".out" 
                    exec_file = output_dir + fname + ".exe"
                    cmd = ' '.join(["riscv32-unknown-linux-gnu-gcc", output_dir + file, "sylib-riscv-linux.a", '-o', exec_file])
                    os.system(cmd)
                    if not os.path.exists(exec_file):
                        record[file] = {"retval": -1, "err_detail": "executing cmd [" + cmd + "] failed, your assmbly can not produce a executable"}
                        continue
                
                    # qemu 
                    cmd = ' '.join(["qemu-riscv32.sh", exec_file])
                    input_file = testcase_dir + fname + ".in"
                    if os.path.exists(input_file):
                        cmd = ' '.join([cmd, "<", input_file])
                    cmd = ' '.join([cmd, ">", output_file])
                    cp = subprocess.run(cmd, shell=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL)
                    with open(output_file, "a") as f:
                        f.write("\n" + str(cp.returncode))

                    # diff
                    cmd = ' '.join(["diff", ref_file, output_file, '-wB'])
                    if is_windows:
                        cmd = cmd.replace('/','\\')
                    cp = subprocess.run(cmd, shell=True, stderr=subprocess.PIPE, stdout=subprocess.DEVNULL)
                    if cp.returncode != 0:
                        record[file] = {"retval": cp.returncode, "err_detail": cp.stderr}
                    else:
                        score += 1
                        record[file] = {"retval": 0}
                    print(file, record[file])
        print("score:",score,"/",total)
    else:
        print("TODO")
        # exit()
        
    return int(score/total * 100)


if __name__ == "__main__":
    assert(len(sys.argv) == 2)
    score_compiler(sys.argv[1])