import tkinter
class EmployeeManagerGUI(object):
    def __init__(self, helperPath = None) -> None:
        self.top = tkinter.Tk()
        self.helperRead(helperPath)
        self.setLayout()
        helperButton = tkinter.Button(self.top, activebackground="red")
        helperButton.pack()
        self.top.mainloop()
        
    def setLayout(self):
        self.top.title('企业员工管理系统')  # 标题
        screenwidth = self.top.winfo_screenwidth()  # 屏幕宽度
        screenheight = self.top.winfo_screenheight()  # 屏幕高度
        width = 1200
        height = 800
        x = int((screenwidth - width) / 2)
        y = int((screenheight - height) / 2)
        self.top.geometry('{}x{}+{}+{}'.format(width, height, x, y))  # 大小以及位置
        
    def helperRead(self, path):
        with open(path, encoding="utf8") as f:
            datas = f.readlines()
        # for row in datas:
        #     print(row.strip("\n "))