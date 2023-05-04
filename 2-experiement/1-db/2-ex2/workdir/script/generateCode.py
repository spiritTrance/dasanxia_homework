def genCode_employeeUpd(ls):
    for s in ls:
        print('''    def update{:s}(self, eid, {:s}):
        \'''
        function
        --------
        update {:s}
        
        params
        ------
        eid: employer's id\
        {:s}: employer's {:s}
        
        returns
        -------
        bool: whether operation valid
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set {:s} = %s where eid = \%s", ({:s}, eid))
            return True
        except:
            return False\n'''.format(s, s, s, s, s, s, s)
        )
def genCode_employeeDel(ls):
    for s in ls:
        print('''    def deleteBy{:s}(self, {:s}):
        \'''
        function
        --------
        delete employee by {:s}
        
        params
        ------
        {:s}: employer's {:s}
        
        returns
        -------
        bool: whether operation valid
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where {:s} = %s", [{:s}])
            return True
        except:
            return False\n'''.format(s, s, s, s, s, s, s, s, s, s)
        )
def genCode_projectUpd(ls):
    for s in ls:
        print('''
    def update{:s}(self, {:s}, projectName):
        \'''
        function
        --------
        update {:s} of table projeect
        
        params
        ------
        {:s}: the {:s} of the given project
        projectName: 涉及更改的项目名
        
        return
        ------
        bool: whether operation valid
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("update project set {:s} = %s where projectname = %s", ({:s}, projectName))
            return True
        except:
            return False\n'''.format(s,s,s,s,s,s,s,s,s,s,s)
        )    
def genCode_projectDel(ls):
    for s in ls:
        print('''
    def deleteBy{:s}(self, {:s}):
        \'''
        function
        --------
        delete project by {:s}

        params
        ------
        {:s}: {:s}'s {:s}

        returns
        -------
        bool: whether operation valid
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from project where {:s} = %s", [{:s}])
            return True
        except:
            return False'''.format(s,s,s,s,s,s,s,s,s,s,s)
        )

def genCode_departmentDel(ls):
    for s in ls:
        print('''
    def deleteBy{:s}(self, {:s}):
        \'''
        function
        --------
        delete department entry by {:s}

        params
        ------
        {:s}: department entry's {:s}

        returns
        -------
        bool: whether operation valid
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from department where {:s} = %s", [{:s}])
            return True
        except:
            return False'''.format(s,s,s,s,s,s,s,s,s,s,s)
        )


# genCode_employeeUpd(["Gender", "EName", "Age", "Job", "DeptName", "Salary", "Ranking"])
# genCode_employeeDel(["EID","Gender", "EName", "Age", "Job", "DeptName", "Salary", "Ranking"])
# genCode_projectUpd(["Deptname","Starttime", "Endtime"])
# genCode_projectDel(["ProjectName","Deptname","Starttime", "Endtime"])
genCode_departmentDel(["EID","DEPTNAME"])


    
    