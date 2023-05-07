class Department(object):
    def __init__(self, deptName, eid = None) -> None:
        self.departmentName = deptName
        self.principalID = eid