import psycopg2
from EmployeeManager.utils.EntityClass import Department, Employee, Project
class Connecter(object):
    def __init__(self, dataBaseName: str, userName: str, pwd: str, host: str, portNumber: str):
        print("连接数据库...")
        self.conn=psycopg2.connect(\
            database=dataBaseName, \
            user=userName, \
            password=pwd, \
            host=host, \
            port=portNumber \
        )
        self.database = dataBaseName
        self.user = userName
        self.password = pwd
        self.host = host
        self.port = portNumber
        print("连接成功！")
    def getConnector(self):
        if self.conn.isolation_level:
            return self.conn
        else:
            self.conn=psycopg2.connect(\
                database=self.database, \
                user=self.user, \
                password=self.password, \
                host=self.host, \
                port=self.port \
            )
        return self.conn
    def __del__(self):
        self.conn.commit()
        self.conn.close()
class EmployeeProcessor(object):
    def __init__(self, conn):
        self.conn = conn
    # inserter 
    def addEmployer(self, employee: Employee):
        '''
        function
        --------
        add employer
        
        params
        ------
        employee: an object of Employee
        
        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            eid = 9
            cur.execute("select max(eid) from employee")
            ans = cur.fetchone()[0]
            cur.execute("INSERT INTO EMPLOYEE VALUES(%s, %s, %s, %s, %s, %s, %s, %s)", \
                (ans + 1, employee.sName, employee.sGender, employee.nAge, \
                employee.job, employee.department, employee.salary, employee.ranking))
            self.conn.commit()
            print("操作成功！")
            return True
        except:
            print("请检查你的输入是否合法！")
            return False
    # updater
    def updateGender(self, eid, Gender):
        '''
        function
        --------
        update Gender

        params
        ------
        eid: employer's id        Gender: employer's Gender

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Gender = %s where eid = \%s", (Gender, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateEName(self, eid, EName):
        '''
        function
        --------
        update EName

        params
        ------
        eid: employer's id        EName: employer's EName

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set EName = %s where eid = \%s", (EName, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateAge(self, eid, Age):
        '''
        function
        --------
        update Age

        params
        ------
        eid: employer's id        Age: employer's Age

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Age = %s where eid = \%s", (Age, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateJob(self, eid, Job):
        '''
        function
        --------
        update Job

        params
        ------
        eid: employer's id        Job: employer's Job

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Job = %s where eid = \%s", (Job, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateDeptName(self, eid, DeptName):
        '''
        function
        --------
        update DeptName

        params
        ------
        eid: employer's id        DeptName: employer's DeptName

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set DeptName = %s where eid = \%s", (DeptName, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateSalary(self, eid, Salary):
        '''
        function
        --------
        update Salary

        params
        ------
        eid: employer's id        Salary: employer's Salary

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Salary = %s where eid = \%s", (Salary, eid))
            self.conn.commit()
            return True
        except:
            return False
    def updateRanking(self, eid, Ranking):
        '''
        function
        --------
        update Ranking

        params
        ------
        eid: employer's id        Ranking: employer's Ranking

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Ranking = %s where eid = \%s", (Ranking, eid))
            self.conn.commit()
            return True
        except:
            return False
    # queryer
    def queryIdByName(self, sName:str):
        '''
        function
        --------
        根据名字查询员工id
        
        params
        ------
        sName: employee's name
        
        returns
        -------
        ans: list of name
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("select eid from employee where ename = %s", [sName])
            ans = cur.fetchall()
        except:
            print("请检查是否为合法输入！")
            ans = []
        finally:
            return ans        
    def queryInfoById(self, eid: int):
        '''
        function
        --------
        根据id查询该员工的信息
        
        params
        ------
        eid: 员工的id号
        
        returns
        -------
        ans: 
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("select * from employee where eid = %s", [eid])
            ans = cur.fetchall()
        except Exception as e:
            print("请检查是否为合法输入！")
            print(e)
            ans = []

        finally:
            return ans  
    def queryInfoByName(self, sName: str):
        '''
        function
        --------
        根据名字查询信息（可能有多个）
        
        params
        ------
        sName: employee's name
        
        returns
        -------
        ans: list of employee's info
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("select * from employee where ename = %s", [sName])
            ans = cur.fetchall()
        except:
            print("请检查是否为合法输入！")
            ans = []
        finally:
            return ans    
    def queryIDNamePairByDept(self, sDeptName: str):
        '''
        function
        --------
        查询一个部门下面有哪些人，及相应eid
        
        params
        ------
        sDeptName: 部门名
        
        returns
        -------
        ans: list of pair of eid and name in the constraint of department name
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("select eid, ename from employee where deptname = %s", [sDeptName])
            ans = cur.fetchall()
        except:
            print("请检查是否为合法输入！")
            ans = []
        finally:
            return ans    
    # deleter
    def deleteByEID(self, EID):
        '''
        function
        --------
        delete employee by EID
        
        params
        ------
        EID: employer's EID
        
        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where EID = %s", [EID])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByGender(self, Gender):
        '''
        function
        --------
        delete employee by Gender

        params
        ------
        Gender: employer's Gender

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Gender = %s", [Gender])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByEName(self, EName):
        '''
        function
        --------
        delete employee by EName

        params
        ------
        EName: employer's EName

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where EName = %s", [EName])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByAge(self, Age):
        '''
        function
        --------
        delete employee by Age

        params
        ------
        Age: employer's Age

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Age = %s", [Age])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByJob(self, Job):
        '''
        function
        --------
        delete employee by Job

        params
        ------
        Job: employer's Job

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Job = %s", [Job])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByDeptName(self, DeptName):
        '''
        function
        --------
        delete employee by DeptName

        params
        ------
        DeptName: employer's DeptName

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where DeptName = %s", [DeptName])
            self.conn.commit()
            return True
        except:
            return False
    def deleteBySalary(self, Salary):
        '''
        function
        --------
        delete employee by Salary

        params
        ------
        Salary: employer's Salary

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Salary = %s", [Salary])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByRanking(self, Ranking):
        '''
        function
        --------
        delete employee by Ranking

        params
        ------
        Ranking: employer's Ranking

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Ranking = %s", [Ranking])
            self.conn.commit()
            return True
        except:
            return False
class ProjectProcessor(object):
    def __init__(self, conn):
        self.conn = conn
    # Create
    def addProject(self, project: Project):
        '''
        function
        --------
        增加项目
        
        params
        ------
        project: object of Project
        
        return
        ------
        bool: whether operation valid

        '''
        try:
            cur = self.conn.cursor()
            cur.execute("INSERT INTO Project VALUES(%s, %s, %s, %s)", \
                (project.projectName, project.departmentName, project.startTime, project.endTime)
            )
            self.conn.commit()
            return True
        except Exception as e:
            print("请检查输入合法性！")
            return False
    # Retrieve
    def queryProjectNameByDepartment(self, sDeptName):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT projectname FROM Project where deptname = %s", [sDeptName])
            ans = cur.fetchall()
        except Exception as e:
            print("请检查输入合法性！")
            ans = []
        finally:
            return ans
    def queryDepartmentNameByProject(self, sProject):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT deptname FROM Project where projectname = %s", [sProject])
            ans = cur.fetchall()
        except Exception as e:
            print("请检查输入合法性！")
            ans = []
        finally:
            return ans
    # Update
    def updateProjectName(self, newName, projectName):
        '''
        function
        --------
        更新项目名字
        
        params
        ------
        newName: 要更换上的新名字
        projectName: 要更改的项目名
        
        return
        ------
        bool: whether operation valid

        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update project set projectname = %s where projectname = %s", (newName, projectName))
            self.conn.commit()
            return True
        except:
            return False
    def updateDeptname(self, Deptname, projectName):
        '''
        function
        --------
        update Deptname of table project

        params
        ------
        Deptname: the Deptname of the given project
        projectName: 涉及更改的项目名

        return
        ------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update project set Deptname = %s where projectname = %s", (Deptname, projectName))
            self.conn.commit()
            return True
        except:
            return False
    def updateStarttime(self, Starttime, projectName):
        '''
        function
        --------
        update Starttime of table project

        params
        ------
        Starttime: the Starttime of the given project
        projectName: 涉及更改的项目名

        return
        ------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update project set Starttime = %s where projectname = %s", (Starttime, projectName))
            self.conn.commit()
            return True
        except:
            return False
    def updateEndtime(self, Endtime, projectName):
        '''
        function
        --------
        update Endtime of table project

        params
        ------
        Endtime: the Endtime of the given project
        projectName: 涉及更改的项目名

        return
        ------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update project set Endtime = %s where projectname = %s", (Endtime, projectName))
            self.conn.commit()
            return True
        except:
            return False
    # Delete    
    def deleteByProjectName(self, ProjectName):
        '''
        function
        --------
        delete project by ProjectName

        params
        ------
        ProjectName: ProjectName's ProjectName

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from project where ProjectName = %s", [ProjectName])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByDeptname(self, Deptname):
        '''
        function
        --------
        delete project by Deptname

        params
        ------
        Deptname: Deptname's Deptname

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from project where Deptname = %s", [Deptname])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByStarttime(self, Starttime):
        '''
        function
        --------
        delete project by Starttime

        params
        ------
        Starttime: Starttime's Starttime

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from project where Starttime = %s", [Starttime])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByEndtime(self, Endtime):
        '''
        function
        --------
        delete project by Endtime

        params
        ------
        Endtime: Endtime's Endtime

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from project where Endtime = %s", [Endtime])
            self.conn.commit()
            return True
        except:
            return False
class DepartmentProcessor(object):
    def __init__(self, conn):
        self.conn = conn
    # Create
    def addDepartment(self, dept: Department):
        '''
        function
        --------
        增加部门
        
        params
        ------
        project: object of Project
        
        return
        ------
        bool: whether operation valid

        '''
        try:
            cur = self.conn.cursor()
            cur.execute("INSERT INTO Department VALUES(%s, %s)", \
                (dept.departmentName, dept.principalID)
            )
            self.conn.commit()
            return True
        except Exception as e:
            return False
    # Retrieve
    def queryCommanderID(self, deptName):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT EID FROM department where deptname = %s", [deptName])
            ans = cur.fetchall()
        except Exception as e:
            ans = []
        finally:
            return ans
    def queryDepartment(self, commanderID):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT DEPTNAME FROM department where EID = %s", [commanderID])
            ans = cur.fetchall()
        except Exception as e:
            ans = []
        finally:
            return ans
    # Update
    def updateCommanderID(self, cmdID, deptName):
        '''
        function
        --------
        update commander id of table department

        params
        ------
        cmdID: the ID of commander
        deptName: correlated department name

        return
        ------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update department set EID = %s where DEPTNAME = %s", (cmdID, deptName))
            self.conn.commit()
            return True
        except:
            return False
    # Delete
    def deleteByEID(self, EID):
        '''
        function
        --------
        delete department entry by EID

        params
        ------
        EID: department entry's EID

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from department where EID = %s", [EID])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByDEPTNAME(self, DEPTNAME):
        '''
        function
        --------
        delete department entry by DEPTNAME

        params
        ------
        DEPTNAME: department entry's DEPTNAME

        returns
        -------
        bool: whether operation valid
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from department where DEPTNAME = %s", [DEPTNAME])
            self.conn.commit()
            return True
        except:
            return False
class Employee_prjProcessor(object):
    def __init__(self, conn):
        self.conn = conn
    # Create
    def addEID_PrjPair(self, eid, prjName):
        try:
            cur = self.conn.cursor()
            cur.execute("INSERT INTO EM_PRJ VALUES(%s, %s)", \
                (eid, prjName)
            )
            self.conn.commit()
            return True
        except:
            return False
    # Retrieve
    def queryJoinPeople(self, prjName):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT EID FROM EM_PRJ where projectName = %s", [prjName])
            ans = cur.fetchall()
            self.conn.commit()
        except Exception as e:
            ans = []
        finally:
            return ans   
    def queryEID(self, eid):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT projectName FROM EM_PRJ where EID = %s", [eid])
            ans = cur.fetchall()
            self.conn.commit()
        except Exception as e:
            ans = []
        finally:
            return ans   
    # Update
    # No opinions provided, please delete and create. (Pairs provided)
    # Delete
    def deleteByEID(self, eid):
        try:
            cur = self.conn.cursor()
            cur.execute("delete from em_prj where eid = %s", [eid])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByPrjName(self, prjName):
        try:
            cur = self.conn.cursor()
            cur.execute("delete from em_prj where projectName = %s", [prjName])
            self.conn.commit()
            return True
        except:
            return False
    def deleteByPair(self, eid, prjName):
        try:
            cur = self.conn.cursor()
            cur.execute("delete from em_prj where eid = %s and projectName = %s", [eid, prjName])
            self.conn.commit()
            return True
        except:
            return False