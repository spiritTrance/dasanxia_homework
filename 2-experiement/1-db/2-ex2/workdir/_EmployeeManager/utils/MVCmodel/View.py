import os
from EmployeeManager.utils.EntityClass import Employee, Project, Department
from EmployeeManager.utils.MVCmodel import Model
class CmdView(object):
    def __init__(self, connector: Model.Connecter):
        self.connector = connector
        try:
            self.employeePrc = Model.EmployeeProcessor(connector.getConnector())
            self.projectPrc = Model.ProjectProcessor(connector.getConnector())
            self.deptPrc = Model.DepartmentProcessor(connector.getConnector())
            self.em_prj = Model.Employee_prjProcessor(connector.getConnector())
            self.showEmployeeInfoView()
        except Exception as e:
            # self.__clearPage()
            print(e)
            print("异常退出")
                
    def __clearPage(self):
        if os.name == 'nt':
            os.system('cls')
        else:
            os.system('clear')

    def showNavigation(self):
        self.__clearPage()
        panel1 = '''
-----------------------------------------------------
  欢迎来到企业职工管理系统，请输入对应数字进行操作！
-----------------------------------------------------
 1 - 职工管理                                        
 2 - 部门管理                                        
 3 - 项目管理                                        
 4 - 退出                                      
-----------------------------------------------------
'''
        panel2 = '''
-----------------------------------------------------
  ERROR!: 请检查你输入的数字是否正确！
  ERROR!: 请检查你输入的数字是否正确！
  ERROR!: 请检查你输入的数字是否正确！
-----------------------------------------------------
 1 - 职工管理                                        
 2 - 部门管理                                        
 3 - 项目管理                                        
 4 - 退出                                      
-----------------------------------------------------
'''
        choice = 100
        print(panel1)
        while not (1 <= choice <= 4):
            choice = int(input().strip(" "))
            if choice == 1:
                self.showEmployeeManagementPanel()
            elif choice == 2:
                a = 3
            elif choice == 3:
                a = 3
            elif choice == 4:
                exit(0)
            else:
                self.__clearPage()
                print(panel2)
                
    def showEmployeeManagementPanel(self):
        self.__clearPage()
        panel1 = '''
-----------------------------------------------------
  请选择您要对职工进行的操作
-----------------------------------------------------
 1 - 增加员工                                        
 2 - 开除员工                                        
 3 - 查询员工信息                                        
 4 - 修改员工信息                                        
 5 - 退出                                      
-----------------------------------------------------
'''
        panel2 = '''
-----------------------------------------------------
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
-----------------------------------------------------
 1 - 增加员工                                        
 2 - 开除员工                                        
 3 - 查询员工信息                                        
 4 - 修改员工信息                                        
 5 - 退出                                      
-----------------------------------------------------
'''
        choice = 100
        print(panel1)
        while not (1 <= choice <= 5):
            choice = int(input().strip(" "))
            if choice == 1:
                a = 1
            if choice == 2:
                self.expireEmployeeView()
            if choice == 3:
                self.showEmployeeInfoView()
            if choice == 4:
                a = 1
            if choice == 5:
                self.showNavigation()
            else:
                print(panel2)
    def showEmployeeInfoView(self):
        panel1 = '''
-----------------------------------------------------
 请输入职工ID：
-----------------------------------------------------
'''
        panel2 = '''
-----------------------------------------------------
 员工ID不存在，请检查后重新输入:
-----------------------------------------------------
'''
        panel3 = '''
-----------------------------------------------------
 ERROR: 请检查输入是否合法！！
 ERROR: 请检查输入是否合法！！
 ERROR: 请检查输入是否合法！！
 请输入职工ID：
-----------------------------------------------------
'''
        choice = 100
        self.__clearPage()
        print(panel1)
        while True:
            choice = input().strip(" ")
            try:
                choice = int(choice)
            except:     # 输入的不是整数
                self.__clearPage()
                print(panel3)
                continue
            ans = self.employeePrc.queryInfoById(choice)
            if (len(ans) == 0):         # 查无此人
                self.__clearPage()
                print(panel2)              
                continue  
            ans = ans[0]
            divLine = "-----------------------------------------------------"
            msg = ["员工ID:\t\t", "员工姓名:\t", "员工性别:\t", \
                "员工年龄:\t", "员工岗位:\t", "员工所属部门:\t", \
                    "员工薪资:\t", "员工绩效:\t"]
            print(divLine)
            for idx, val in enumerate(ans):
                print(msg[idx], end = "")
                if val is None:
                    print("Null")
                else:
                    print(val)
            print(divLine)
            os.system("pause")
            break
        self.showEmployeeManagementPanel()
    def expireEmployeeView(self):
        panel = '''
-----------------------------------------------------
 请输入要开除的员工ID：     
-----------------------------------------------------
'''
        panel2 = '''
-----------------------------------------------------
 员工ID不存在，请检查后重新输入:
-----------------------------------------------------
'''
        panel3 = '''
-----------------------------------------------------
 ERROR: 请检查输入是否合法！！
 ERROR: 请检查输入是否合法！！
 ERROR: 请检查输入是否合法！！
 请输入职工ID：
-----------------------------------------------------
'''
        self.__clearPage()
        print(panel)
        while True:
            eid = input().strip()
            try:
                eid = int(eid)
            except:
                self.__clearPage()
                print(panel2)
                continue
            if self.employeePrc.deleteByEID(eid):
                self.__clearPage()
                print("已成功删除职工{:d}".format(eid))
                os.system("pause")
                break
            else:
                self.__clearPage()
                print(panel2)
        self.showEmployeeManagementPanel()
    def addEmployeeView(self):
        pass
                
                
                
                