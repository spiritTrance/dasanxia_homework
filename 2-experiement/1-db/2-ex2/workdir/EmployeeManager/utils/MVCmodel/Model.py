import psycopg2
from EmployeeManager.utils.EntityClass import Department, Employee, Project
class Connecter(object):
    def __init__(self, dataBaseName: str, userName: str, pwd: str, host: str, portNumber: str) -> None:
        print("连接数据库...")
        self.conn=psycopg2.connect(\
            database=dataBaseName, \
            user=userName, \
            password=pwd, \
            host=host, \
            port=portNumber \
        )
        print("连接成功！")
        
    def getConnector(self):
        return self.conn
    
    def __del__(self):
        self.conn.commit()
        self.conn.close()

class EmployeeProcessor(object):
    def __init__(self, conn) -> None:
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
        None
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
        except:
            print("请检查你的输入是否合法！")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Gender = %s where eid = %s", (Gender, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set EName = %s where eid = %s", (EName, eid))
        except Exception as e:
            print(e)
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Age = %s where eid = %s", (Age, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Job = %s where eid = %s", (Job, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set DeptName = %s where eid = %s", (DeptName, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Salary = %s where eid = %s", (Salary, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set Ranking = %s where eid = %s", (Ranking, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return
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
            ans = None
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where EID = %s", [EID])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Gender = %s", [Gender])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where EName = %s", [EName])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
    def deleteByAge(self, Age):
        '''
        function
        --------
        delete employee whose age bigger than given age

        params
        ------
        Age: employer's Age

        returns
        -------
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Age > %s", [Age])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Job = %s", [Job])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where DeptName = %s", [DeptName])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where Ranking = %s", [Ranking])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return
    
class ProjectProcessor(object):
    def __init__(self, conn) -> None:
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
        None
        '''
        try:
            cur = self.conn.cursor()
            cur.execute("INSERT INTO Project VALUES(%s, %s, %s, %s)", \
                (project.projectName, project.departmentName, project.startTime, project.endTime)
            )
        except Exception as e:
            print("请检查输入合法性！")
        finally:
            return
    # Retrieve
    def getProjectNameByDepartment(self, sDeptName):
        try:
            cur = self.conn.cursor()
            cur.execute("SELECT projectname FROM Project where deptname = %s", [sDeptName])
            ans = cur.fetchall()
        except Exception as e:
            print("请检查输入合法性！")
            ans = []
        finally:
            return ans
    def getDepartmentNameByProject(self, sProject):
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
    # FIXME TODO here
    # Delete    
class DepartmentProcessor(object):
    def __init__(self) -> None:
        pass

class Employee_prjProcessor(object):
    def __init__(self) -> None:
        pass
    
    