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
        None
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("update employee set {:s} = %s where eid = \%s", ({:s}, eid))
        except:
            print("请检查你的输入是否合法，以及用户是否存在")
        finally:
            return\n'''.format(s, s, s, s, s, s, s)
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
        None
        \'''
        try:
            cur = self.conn.cursor()
            cur.execute("delete from employee where {:s} = %s", [{:s}])
            print("删除成功！")
        except:
            print("请检查你的输入是否合法!")
        finally:
            return\n'''.format(s, s, s, s, s, s, s, s, s, s)
        )
    
# genCode_employeeUpd(["Gender", "EName", "Age", "Job", "DeptName", "Salary", "Ranking"])
genCode_employeeDel(["EID","Gender", "EName", "Age", "Job", "DeptName", "Salary", "Ranking"])