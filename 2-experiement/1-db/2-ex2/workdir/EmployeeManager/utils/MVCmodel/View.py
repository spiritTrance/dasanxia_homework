import os
from EmployeeManager.utils.EntityClass import Employee, Project, Department
from EmployeeManager.utils.MVCmodel import Model
import traceback
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
  ERROR!: 请检查你输入的数字是否正确！'''
        while True:
            print(panel1)
            try:
                choice = int(input().strip(" "))
                if choice == 1:
                    self.showEmployeeManagementPanel()
                elif choice == 2:
                    self.departmentManagePanel()
                elif choice == 3:
                    self.projectManagePanel()
                elif choice == 4:
                    self.__clearPage()
                    break
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
        panel2 = '''-----------------------------------------------------
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！'''
        while True:
            print(panel1, end="")
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
                    self.__clearPage()
                    break
                else:
                    raise Exception("Invalid input")
            except Exception as e:
                self.__clearPage()
                traceback.print_exc()
                print(panel2, end="")
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
            self.__clearPage()
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
                self.__clearPage()
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
                self.__clearPage()
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
                    isValid = self.employeePrc.updateEName(eid, s)
                elif idx == 2:
                    isValid = self.employeePrc.updateAge(eid, int(s))
                elif idx == 3 and s in ['M', 'F']:
                    isValid = self.employeePrc.updateGender(eid, s)
                elif idx == 4:
                    s = float(s)
                    isValid = self.employeePrc.updateSalary(eid, s)
                elif idx == 5 and s in deptName_prc:
                    isValid = self.employeePrc.updateDeptName(eid, s)
                elif idx == 6 and s in ['A', 'B', 'C', 'D', '-', 'U']:
                    isValid = self.employeePrc.updateRanking(eid, s)
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
 6 - 查询该部门下已有的职员                                      
 7 - 退出                                      
-----------------------------------------------------
'''
        panel2 = '''-----------------------------------------------------
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！'''        
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
                    self.queryDeptEmployeeView()
                elif a == 7:
                    self.__clearPage()
                    return
                else:
                    raise Exception("Invalid input")
            except:
                self.__clearPage()
                print(panel2, end="")
    #       1 - 查询已有部门
    def queryDeptNameView(self):
        self.__clearPage()
        lst = self.getDepartmentNameList()
        print(self.__longDiv)
        print("查询到的现有部门有：")
        for val in lst:
            print("\t",val)
        print(self.__longDiv)
        os.system("pause")
        self.__clearPage()
    #       2 - 增设部门                                        
    def addDepartmentView(self):
        self.__clearPage()
        dept_lst = self.getDepartmentNameList()
        while True:
            print(self.__longDiv)
            print("请输入你想添加的部门，若无想添加的部门，\n请直接输入回车返回:")
            print(self.__longDiv)
            s = input().strip(" ")
            try:
                if len(s) == 0:
                    self.__clearPage()
                    return
                elif s in dept_lst:
                    raise Exception("No duplicate department name")
                dept = Department.Department(s)
                self.deptPrc.addDepartment(dept)
            except:
                print(self.__longDiv)
                print("部门名不能太长，空或重复！")
                os.system("pause")
                self.__clearPage()
                return
            print("添加成功！")
            os.system("pause")
            self.__clearPage()
            return
    #       3 - 解散部门                                        
    def deleteDepartmentView(self):
        self.__clearPage()
        while True:
            print(self.__longDiv)
            print("请输入希望删除的部门名，若无想删除的部门，\n请直接回车返回。\n[注意]:在解散部门前，请确保该部门下的所有员工均已安\n排好，否则该部门下的员工将会一并被解雇！")
            print(self.__longDiv)
            try:
                s = input().strip(" ")
                isDel = self.deptPrc.deleteByDEPTNAME(s)
                if len(s) == 0:
                    self.__clearPage()
                    return
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
        self.__clearPage()
        while True:
            print(self.__longDiv)
            print("请输入相应部门名，查询负责人的ID号：")
            print(self.__longDiv)
            try:
                s = input().strip(" ")
                if len(s) == 0:
                    self.__clearPage()
                    return
                ans = self.deptPrc.queryCommanderID(s)
                self.__clearPage()
                print(self.__longDiv)
                ans = ans[0][0]
                if ans is None:
                    print("该部门暂时没有负责人！")
                else:
                    print("该部门的负责人ID为: ", ans)
                os.system("pause")
                self.__clearPage()
                return
            except:
                self.__clearPage()
                print(self.__longDiv)
                print("请检查输入的部门名是否存在！")
    #       5 - 更改部门负责人   
    def changeCmdIDView(self):
        self.__clearPage()
        while True:
            print(self.__longDiv)
            print("请先后输入部门名以及EID号，中间以空格分隔：")
            print(self.__longDiv)
            try:
                userInput = input().strip(" ")
                s, eid = userInput.split(" ")
                s = s.strip(" ")
                eid = int(eid.strip(" "))
                if not self.deptPrc.updateCommanderID(eid, s):
                    raise Exception("Invalid input")
                print("修改成功！")
                os.system("pause")
                self.__clearPage()
                return
            except Exception as e:
                if len(userInput) == 0:
                    self.__clearPage()
                    return
                self.__clearPage()
                print(e)
                print(self.__longDiv)
                print("请检查输入的EID，部门名是否合法以及存在！")
            #       5 - 更改部门负责人   
    #       6 - 查询该部门下的所有人和eid
    def queryDeptEmployeeView(self):
        self.__clearPage()
        while True:
            print(self.__longDiv)
            print("请输入部门名，以查询该部门下的职工EID和姓名：")
            print(self.__longDiv)
            try:
                userInput = input().strip(" ")
                if len(userInput) == 0:
                    self.__clearPage()
                    return
                ans = self.employeePrc.queryIDNamePairByDept(userInput)
                self.__clearPage()
                print(self.__longDiv)
                if len(ans) == 0:
                    print("该部门下暂无人员...请检查部门名输入是否存在问题")
                    os.system("pause")
                    return
                else:
                    print("查询到的员工有：")
                    print("EID\t\tName")
                    for pair in ans:
                        print(pair[0], pair[1], sep="\t\t")
                os.system("pause")
                self.__clearPage()
                return
            except Exception as e:
                self.__clearPage()
                print(e)
                print(self.__longDiv)
                print("请检查输入的部门名是否合法以及存在！")
            
    
    
    
    # 3 - 项目管理
    def projectManagePanel(self):
        panel1 = '''
-----------------------------------------------------
  请选择您要对项目进行的操作
-----------------------------------------------------
 1 - 新增项目                                        
 2 - 删除项目                                        
 3 - 查询已有项目信息                                        
 4 - 查询现有项目                                        
 5 - 为项目新增参与人                                        
 6 - 为项目删除相关参与人                                        
 7 - 为项目更改负责部门                                        
 8 - 查看项目参与人
 9 - 退出                                      
-----------------------------------------------------
'''
        panel2 = '''-----------------------------------------------------
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！
  ERROR: 请检查输入！！！'''
        while True:
            self.__clearPage()
            print(panel1)
            try:
                idx = int(input().strip(" "))
                if idx == 1:
                    self.addProjectView()
                elif idx == 2:
                    self.deleteProjectView()
                elif idx == 3:
                    self.queryGivenProjectInfoView()
                elif idx == 4:
                    self.queryAllProjectNameView()
                elif idx == 5:
                    self.addNewParticipatorView()
                elif idx == 6:
                    self.deleteParticipatorView()
                elif idx == 7:
                    self.updateCmdDeptView()
                elif idx == 8:
                    self.queryPaticipantsView()
                elif idx == 9:
                    self.__clearPage()
                    return
            except:
                self.__clearPage()
                print(panel2)
                
    #       1 - 新增项目  
    def addProjectView(self):
        dept_list = self.getDepartmentNameList()
        while True:
            self.__clearPage()
            print(self.__longDiv)
            print('''
请依次输入项目名，负责部门名，起始日期和终止日期，其中：
- 四个输入项以空格隔开
- 负责部门必须已存在
- 日期以yyyymmdd的格式输入，如20230809
- 若无需输入，则直接回车退出
                  ''')
            print(self.__longDiv)
            try:
                userIn = input()
                prjName, dept, st, ed = userIn.split(" ")
                if not (self.checkDateValid(st) and self.checkDateValid(ed)):
                    raise Exception("Invalid date")
                if not dept in dept_list:
                    raise Exception("No such department")
                project = Project.Project(
                    prjName=prjName, \
                    deptName=dept, \
                    startTime=st, \
                    endTime=ed
                )
                if not self.projectPrc.addProject(project):
                    raise Exception("Commit Error")
                self.__clearPage()
                print("创建成功！")
                os.system("pause")
                break
            except:
                if len(userIn) == 0:
                    self.__clearPage()
                    return
                self.__clearPage()
                print(self.__longDiv)
                print("请检查输入是否符合要求！")
    #       2 - 删除项目
    def deleteProjectView(self):
        prj_lst = self.getProjectNameList()
        while True:
            self.__clearPage()
            print(self.__longDiv)
            print("请输入需要删除的项目名：")
            print(self.__longDiv)
            try:
                s = input().strip(" ")
                if len(s) == 0:
                    self.__clearPage()
                    return
                if s not in prj_lst:
                    raise Exception("No such project")
                if not self.projectPrc.deleteByProjectName(s):
                    raise Exception("delete failed")
                self.__clearPage()
                print(self.__longDiv, "删除成功", sep="\n")
                os.system("pause")
                break
            except:
                self.__clearPage()
                print(self.__longDiv, "请检查是否存在该项目：", sep="\n")
                os.system("pause")
    #       3 - 查询已有项目信息     
    def queryGivenProjectInfoView(self):
        while True:
            self.__clearPage()
            print(self.__longDiv)
            print("请输入需要查询的项目名：")
            print(self.__longDiv)
            try:
                s = input().strip(" ")
                if len(s) == 0 :
                    return
                ans = self.projectPrc.queryInfoByProject(s)
                if len(ans) == 0 :
                    return
                self.__clearPage()
                print(self.__longDiv, "查询到该项目信息：", sep = "\n")
                ans = ans[0]
                print("项目名:\t\t",ans[0])
                print("负责部门:\t",ans[1])
                print("起始时间:\t",ans[2])
                print("终止时间:\t",ans[3])
                print(self.__longDiv)
                os.system("pause")
                self.__clearPage()
                break
            except:
                self.__clearPage()
                print(self.__longDiv, "请检查是否存在该项目。", sep="\n")
                os.system("pause")
    #       4 - 查询现有项目     
    def queryAllProjectNameView(self):
        self.__clearPage()
        try:
            ans = self.projectPrc.queryAllProjectName()
            if len(ans) == 0:
                raise Exception("query exception")
            print(self.__longDiv, "查询到如下项目：", sep="\n")
            for entry in ans:
                print("\t", entry[0])
            print(self.__longDiv)
        except:
            print("暂无项目存在")
        finally:
            os.system("pause")
            self.__clearPage()
            return
    #       5 - 为项目新增参与人  
    def addNewParticipatorView(self):
        while True:
            self.__clearPage()
            print(self.__longDiv, "请依次输入项目名，职工EID，中间以空格分隔", self.__longDiv,sep='\n')
            try:
                userIn = input().strip(" ")
                prjName, eid = userIn.split(" ")
                eid = str(int(eid))
                if not self.em_prjPrc.addEID_PrjPair(eid, prjName):
                    raise Exception("upd failed")
                self.__clearPage()
                print(self.__longDiv, "添加成功！", self.__longDiv, sep="\n")
            except Exception as e:
                if len(userIn) == 0:
                    self.__clearPage()
                    return
                else:
                    self.__clearPage()
                    print(e)
                    print(self.__longDiv, "请检查输入是否合法，或是否已加入！", self.__longDiv, sep = "\n")
            finally:
                os.system("pause")
    #       6 - 为项目删除相关参与人  
    def deleteParticipatorView(self):
        while True:
            self.__clearPage()
            print(self.__longDiv, "请依次输入项目名，职工EID，中间以空格分隔。若不删除，直接按回车退出。", self.__longDiv,sep='\n')
            try:
                userIn = input().strip(" ")
                if len(userIn) == 0:
                    self.__clearPage()
                    return
                prjName, eid = userIn.split(" ")
                eid = int(eid)
                if not self.em_prjPrc.deleteByPair(eid, prjName):
                    raise Exception("invalid delete")
                self.__clearPage()
                print(self.__longDiv, "删除成功！", self.__longDiv, sep="\n")
            except:
                if len(userIn) == 0:
                    self.__clearPage()
                else:
                    self.__clearPage()
                    print(self.__longDiv, "请检查输入是否合法！", self.__longDiv, sep = "\n")
            finally:
                os.system("pause")
    #       7 - 为项目更改负责部门  
    def updateCmdDeptView(self):
        deptList = self.getDepartmentNameList()
        prjList = self.getProjectNameList()
        while True:
            self.__clearPage()
            print(self.__longDiv, "请依次输入项目名，部门名，中间以空格分隔", self.__longDiv,sep='\n')
            try:
                userIn = input().strip(" ")
                prjName, deptName = userIn.split(" ")
                if not ((prjName in prjList) and (deptName in deptList)):
                    raise Exception("No Existence")
                if not self.projectPrc.updateDeptname(deptName, prjName):
                    raise Exception("Commit Error")
                self.__clearPage()
                print(self.__longDiv, "更改成功！", self.__longDiv, sep="\n")
            except Exception as e:
                if len(userIn) == 0:
                    self.__clearPage()
                    return
                else:
                    # self.__clearPage()
                    print(e)
                    print(self.__longDiv, "请检查输入是否合法！", self.__longDiv, sep = "\n")
            finally:
                os.system("pause")
    #       8 - 查看项目参与人  
    def queryPaticipantsView(self):
        prjList = self.getProjectNameList()
        while True:
            self.__clearPage()
            print(self.__longDiv, "请输入项目名：", self.__longDiv, sep = "\n")
            try:
                prjName = input().strip(" ")
                if not prjName in prjList:
                    raise("No such projcet")
                ans = self.em_prjPrc.queryJoinPeople(prjName)
                self.__clearPage()
                if len(ans) == 0:
                    print(self.__longDiv, "暂无人员参与该项目", self.__longDiv, sep="\n")
                else:
                    print(self.__longDiv)
                    print("查询到如下人员的EID: ")
                    for i in ans:
                        print(i[0], end="\t")
                    print("\n",self.__longDiv)
            except Exception as e:
                self.__clearPage()
                if len(prjName) != 0:
                    print(self.__longDiv,"请检查项目是否存在！", self.__longDiv, sep="\n")
                print(e)
            finally:
                os.system("pause")
                self.__clearPage()
                return
                
    # auxilary function    
    def getDepartmentNameList(self):
        deptName_prc = self.deptPrc.queryDepartmentList()
        ans = [idx[0] for idx in deptName_prc]
        return ans
    def isdigit(self, s):
        try:
            a = int(s)
            return a > 0
        except:
            return False
    def checkDateValid(self, s):
        if len(s) != 8:
            return False
        year = s[0:4]
        month = s[4:6]
        day = s[6:]
        if not (self.isdigit(year) \
            and self.isdigit(month) \
            and self.isdigit(day)):
            return False
        year, month, day= int(year), int(month), int(day)
        if not 1 <= month <= 12:
            return False
        dayQuery = [-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
        if not (year % 4 != 0 and year % 400 != 0):
            dayQuery[2] = 29
        return 1 <= day <= dayQuery[month]
    def getProjectNameList(self):
        ans = self.projectPrc.queryAllProjectName()
        lst = [i[0] for i in ans]
        return lst
    
    
    