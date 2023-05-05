class Employee(object):
    def __init__(self, ename, gender, age = None, job = None, deptName = None, salary = None, ranking = None):
        self.sName = ename
        self.sGender = gender
        self.nAge = age
        self.job = job
        self.department = deptName
        self.salary = salary
        self.ranking = ranking
    def getName(self):
        return self.sName
    def getGender(self):
        return self.sGender
    def getAge(self):
        return self.nAge
    def getJob(self):
        return self.job
    def getDeptName(self):
        return self.department
    def getSalary(self):
        return self.salary
    def getRanking(self):
        return self.ranking