'''
doc
'''
from EmployeeManager.utils.EntityClass import Department, Employee, Project
from EmployeeManager.utils.MVCmodel import Model, View
from configparser import ConfigParser

PROJECT_NAME = "EmployeeManager"
CONFIG_PATH = "./{:s}/config/configs.ini".format(PROJECT_NAME)

if __name__ == '__main__':
    # 读取配置文件
    conf = ConfigParser()
    conf.read(CONFIG_PATH)
    # 连接数据库
    connecter = Model.Connecter(\
        dataBaseName=conf["databaseSetting"]["dataBaseName"], \
        userName=conf["databaseSetting"]["userName"] ,\
        pwd=conf["databaseSetting"]["pwd"] ,\
        host=conf["databaseSetting"]["host"], \
        portNumber=conf["databaseSetting"]["portNumber"]
    )
    # e = Employee.Employee("黄昊", "M", 20, "搞事", "advertise", 15000, "S")
    # prc = Model.Employee_prjProcessor(connecter.getConnector())
    # prc.updateEName(10, "傻逼")
    # prc.addEID_PrjPair(11, "对于摸鱼需求的市场调研")    
    # prc.addEID_PrjPair(9, "对于摸鱼需求的市场调研")    
    # prc.deleteByPair(9, "对于摸鱼需求的市场调研")    
    # cur = connecter.getConnector().cursor()
    # cur.execute("SELECT * from EM_PRJ")
    # ans = cur.fetchall()
    # print(ans)
    # GUI图形界面
    cmdView = View.CmdView(connector=connecter)
    