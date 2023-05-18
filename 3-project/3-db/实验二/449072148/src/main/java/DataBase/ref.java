package DataBase;

import java.io.Serializable;
import java.util.HashMap;


public class ref implements Serializable {
    public String tableName = "";
    public HashMap<String, String> r;

    public ref(String tableName) {
        this.tableName = tableName;
        r = new HashMap<String, String>();
    }

    public static void saveRef(ref r) throws Exception {
        tool.saveObject(r, r.tableName + "\\ref.txt");
    }

    public static ref loadRef(String tableName) throws Exception {
        return (ref) tool.readObject(tableName);
    }

    // 下文的id带有.txt
    public void insert(String pk, String id) {
        r.put(pk, id);
    }

    public String getID(String pk) {
        if (r.get(pk) != null) {
            return r.get(pk);
        } else {
            return "-1";
        }
    }
}
