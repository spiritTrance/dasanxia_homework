package DataBase;

import javafx.util.Pair;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;

public class tableManager implements Serializable {
    public String folderPath;
    public HashMap<String, ArrayList<Pair<String, String>>> allTables = new HashMap<String, ArrayList<Pair<String, String>>>();
    //public HashMap<String,HashMap<String,String>> allTables = new HashMap<String, HashMap<String, String>>();

    public tableManager(String folderPath) {
        this.folderPath = folderPath;
    }

    public static void writeTables(tableManager t) throws Exception {
        tool.saveObject(t, t.folderPath + "\\tableManager.txt");
    }

    public static tableManager loadTables(String folderPath) throws Exception {
        return (tableManager) tool.readObject(folderPath);
    }

    public void insert(String tableName, ArrayList<Pair<String, String>> params) {
        allTables.put(tableName, params);
    }

}
