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
            self.em_prjPrc = Model.Employee_prjProcessor(connector.getConnector())
            self.showNavigation()
        except Exception as e:
            # self.__clearPage()
            print(e)
            print("异常退出")
           
    @property
    def __longDiv(self):
        return "-----------------------------------------------------"
                
    def __clearPage(self):
        if os.name == 'nt':
            os.system('cls')
        else:
            os.system('clear')
    # 0 - 总导航
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
        while True:
            print(panel1)
            try:
                choice = int(input().strip(" "))
                if choice == 1:
                    self.showEmployeeManagementPanel()
                elif choice == 2:
                    self.departmentManagePanel()
                elif choice == 3:
                    a = 3
                elif choice == 4:
                    self.__clearPage()
                    exit(0)
                else:
                    raise Exception("Invalid input")
            except:
                self.__clearPage()
                print(panel2)
                    
    
    
    # 1 - employee manage      
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
 5 - 查询员工参与的项目                                        
 6 - 退出                                      
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
        while True:
            print(panel1)
            try:
                choice = int(input().strip(" "))
                if choice == 1:
                    self.addEmployeeView()
                elif choice == 2:
                    self.expireEmployeeView()
                elif choice == 3:
                    self.showEmployeeInfoView()
                elif choice == 4:
                    self.editEmployeeView()
                elif choice == 5:
                    self.showEmployeeProject()
                elif choice == 6:
                    break
                else:
                    raise Exception("Invalid input")
            except:
                self.__clearPage()
                print(panel2)
    #       3 - 查询员工信息
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
    #       2 - 开除员工                                        
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
    #       1 - 增加员工
    def addEmployeeView(self):
        self.__clearPage()
        panel = '''-----------------------------------------------------
     请输入要增加员工的信息，按照姓名，性别，
年龄，岗位，工作部门，薪资和绩效评级的顺序输
入。其中请注意：
- 每项输入中间以空格隔开
- 性别输入单个字母代替，其中M - 男；F - 女
- 绩效评级分为A-D四个等级，无评级则用短横
  线'-'代替    
- 输入完成后，按下回车键表示确认 
- 确保部门名是已有的部门名
- 如果不想新加，请输入-1退出
-----------------------------------------------------'''
        panel2 = '''-----------------------------------------------------
ERROR: 请检查输入是否合法！以下是常见的错误，比如：
- 输入项目不全或超过7
- 年龄输入不为1-100以内的整数
- 薪水不为大于0的浮点数
- 部门名是已有的部门名
如果你确信没有错误，则可能是网络的原因，请再试一次'''
        panel3 = '''-----------------------------------------------------
        尚未存在部门，请先添加部门！
-----------------------------------------------------'''
        deptName_prc = self.getDepartmentNameList()
        if len(deptName_prc) == 0:
            print(panel3)
            os.system("pause")
            self.__clearPage()
            return
        while True:
            print(panel)
            print("已有的部门为：")
            for val in deptName_prc:
                print("\t", val)
            print(self.__longDiv)
            inputInfo = input().split(" ")
            inputInfo = list(filter(lambda x: len(x) != 0, inputInfo))
            inputInfo = list(map(lambda x: x.strip(" "), inputInfo))
            if inputInfo[0] == '-1':
                self.__clearPage()
                return
            if len(inputInfo) != 7:
                self.__clearPage()
                print(panel2)
                continue
            sName, gender, age, job, dept, salary, ranking = inputInfo
            try:
                salary = float(salary)
                age = int(age)
                if (gender not in ['M', 'F']) or (salary < 0) or (ranking not in ['A', 'B', 'C', 'D', '-', 'U']):
                    self.__clearPage()
                    print(panel2)
                    continue
            except:
                self.__clearPage()
                print(panel2)
                continue
            print(sName, gender, age, job, dept, salary, ranking)
            e = Employee.Employee(
                ename=sName,
                gender=gender,
                age=age,
                job=job,
                deptName=dept,
                salary=salary,
                ranking=ranking
            )
            isSubmitSucceed, eid = self.employeePrc.addEmployer(e)
            self.__clearPage()
            if isSubmitSucceed:
                print('''
提交成功！员工的eid为: {:d}                  
                  '''.format(int(eid)))
                os.system("pause")
                break
            else:
                print(panel2)
    #       4 - 修改员工信息
    def editEmployeeView(self):
        self.__clearPage()
        panel1 = '''-----------------------------------------------------
        请输入员工eid：
-----------------------------------------------------'''
        panel2 = '''-----------------------------------------------------
        eid不存在或不合法！'''
        panel3 = '''-----------------------------------------------------
请输入一个数字和修改内容，数字和修改内容之间隔一个空格:
1 - 姓名
2 - 年龄
3 - 性别
4 - 薪资
5 - 所属部门
6 - 评级

其中请注意：
- 每项输入中间以空格隔开
- 性别输入单个字母代替，其中M - 男；F - 女
- 绩效评级分为A-D四个等级，无评级则用短横
  线'-'代替    
- 输入完成后，按下回车键表示确认 
- 确保部门名是已有的部门名
-----------------------------------------------------'''
        panel4 = '''-----------------------------------------------------
ERROR: 请检查输入是否合法！以下是常见的错误，比如：
- 输入项目不全或超过7
- 年龄输入不为1-100以内的整数
- 薪水不为大于0的浮点数
- 部门名是已有的部门名
如果你确信没有错误，则可能是网络的原因，请再试一次'''
        deptName = self.deptPrc.queryDepartmentList()
        deptName_prc = [val[0] for val in deptName]
        while True:
            print(panel1)
            try:
                eid = int(input().strip(" "))
                info = self.employeePrc.queryInfoById(eid)
                if len(info) == 0:
                    raise Exception("Invalid EID")
            except Exception as e:
                self.__clearPage()
                print(panel2)
                continue
            break
        self.__clearPage()
        while True:
            print(panel3)
            print("以下是已经存在的部门：")
            for val in deptName_prc:
                print(val)
            print(self.__longDiv)
            try:
                idx, s = input().split(" ")
                idx = int(idx.strip(" "))
                s = s.strip(" ")
                if idx == 1:
                    self.employeePrc.updateEName(eid, s)
                elif idx == 2:
                    self.employeePrc.updateAge(int(s))
                elif idx == 3 and s in ['M', 'F']:
                    self.employeePrc.updateGender(s)
                elif idx == 4:
                    s = float(s)
                    self.employeePrc.updateSalary(s)
                elif idx == 5 and s in deptName_prc:
                    self.employeePrc.updateDeptName(s)
                elif idx == 6 and s in ['A', 'B', 'C', 'D', '-', 'U']:
                    self.employeePrc.updateRanking(s)
                else:
                    raise Exception("Invalid input")
                print("更新成功！")
                os.system("pause")
                self.__clearPage()
                return
            except Exception as e:
                self.__clearPage()
                print(e)
                print(panel4)
                continue
    #       5 - 查询员工参与的项目
    def showEmployeeProject(self):
        self.__clearPage()
        panel1 = '''-----------------------------------------------------
        请输入员工eid：
-----------------------------------------------------'''
        panel2 = '''-----------------------------------------------------
        eid不存在或不合法！'''
        while True:
            print(panel1)
            try:
                a = input().strip()
                self.employeePrc.queryInfoById(int(a))
                ans = self.em_prjPrc.queryEID(int(a))
                self.__clearPage()
                print(self.__longDiv)
                if len(ans) == 0:
                    print("暂未查询到该员工参与项目")
                else:
                    print("该员工参与的项目有：")
                    for i in ans:
                        print("\t", i[0])
                os.system("pause")
                self.__clearPage()
                return
            except Exception as e:
                self.__clearPage()
                print(e)
                print(panel2)
                continue
            
    
      
    # 2 - department manage
    def departmentManagePanel(self):
        self.__clearPage()
        panel1 = '''
-----------------------------------------------------
  请选择您要对部门进行的操作
-----------------------------------------------------
 1 - 查询已有部门                                        
 2 - 增设部门                                        
 3 - 解散部门                                        
 4 - 查询部门负责人ID                                        
 5 - 更改部门负责人                                        
 6 - 退出                                      
-----------------------------------------------------
'''
        panel2 = '''
-----------------------------------------------------
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
'''        
        while True:
            try:
                print(panel1)
                a = int(input().strip(" "))
                if a == 1:
                    self.queryDeptNameView()
                elif a == 2:
                    self.addDepartmentView()
                elif a == 3:
                    self.deleteDepartmentView()
                elif a == 4:
                    self.queryCmdIDView()
                elif a == 5:
                    self.changeCmdIDView()
                elif a == 6:
                    self.__clearPage()
                    return
                else:
                    raise Exception("Invalid input")
            except:
                self.__clearPage()
                print(panel2)
    #       1 - 查询已有部门
    def queryDeptNameView(self):
        lst = self.getDepartmentNameList()
        print(self.__longDiv)
        print("查询到的现有部门有：")
        for val in lst:
            print(val)
        print(self.__longDiv)
        os.system("pause")
    #       2 - 增设部门                                        
    def addDepartmentView(self):
        while True:
            print(self.__longDiv)
            print("请输入你想添加的部门:")
            print(self.__longDiv)
            s = input().strip(" ")
            try:
                dept = Department.Department(s)
                self.deptPrc.addDepartment(dept)
            except:
                print(self.__longDiv)
                print("部门名不能太长或重复！")
                os.system("pause")
                continue
            print("添加成功！")
            return
    #       3 - 解散部门                                        
    def deleteDepartmentView(self):
        while True:
            print(self.__longDiv)
            print("请输入希望删除的部门名：")
            print(self.__longDiv)
            try:
                s = input().strip(" ")
                isDel = self.deptPrc.deleteByDEPTNAME(s)
                if isDel:
                    self.__clearPage()
                    print(self.__longDiv)
                    print("删除成功！")
                    print(self.__longDiv)
                    os.system("pause")
                    self.__clearPage()
                    return
                else:
                    raise Exception("Delete Failed")
            except:
                print("请检查所删除的部门是否存在！")
                os.system("pause")
                self.__clearPage()
    #       4 - 查询部门负责人ID                                        
    def queryCmdIDView(self):
        pass
    #       5 - 更改部门负责人   
    def changeCmdIDView(self):
        pass
    
    
      
    # auxilary function    
    def getDepartmentNameList(self):
        deptName_prc = self.getDepartmentNameList()
        return deptName_prc
    