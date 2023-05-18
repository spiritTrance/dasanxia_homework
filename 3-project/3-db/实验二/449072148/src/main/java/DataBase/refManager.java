package DataBase;

import java.io.File;
import java.util.HashMap;

public class refManager {
    public String dbName;
    public HashMap<String, ref> allRef;

    public refManager(String dbName) {
        allRef = new HashMap<String, ref>();
        this.dbName = dbName;
    }

    // 加载此数据库所有表的索引文件
    public static refManager loadAllRef(String dbName) throws Exception {
        refManager ret = new refManager(dbName);
        File file = new File(dbName);
        File[] files = file.listFiles();
        for (File f : files) {
            if (f.isDirectory()) {
                ret.allRef.put(f.getName(), ref.loadRef(dbName + "\\" + f.getName() + "\\ref.txt"));
            }
        }
        return ret;
    }

    // 存放所有表的索引文件
    public static void saveAllRef(refManager rManager) throws Exception {
        for (ref f : rManager.allRef.values()) {
            if (f != null) {
                ref.saveRef(f);
            }
        }
    }

    public ref getRef(String tableName) {
        return allRef.get(tableName);
    }
}
