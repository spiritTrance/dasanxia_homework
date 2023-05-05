class Project(object):
    def __init__(self, prjName, deptName, startTime = None, endTime = None) -> None:
        self.projectName = prjName
        self.departmentName = deptName
        self.startTime = startTime
        self.endTime = endTime