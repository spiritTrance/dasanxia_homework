package DataBase;

import java.io.File;
import java.util.HashMap;

public class blockManager {
    private static final int MAX = 100;
    public String path;
    public HashMap<String, HashMap<String, Block>> blocks;
    private int cnt = 0;

    public blockManager(String path) {
        this.path = path;
        this.blocks = new HashMap<String, HashMap<String, Block>>();
    }

    // TODO 加载整个数据库中所有表的所有block
    public static blockManager loadDBAllBlock(String dbName) throws Exception {
        blockManager ret = new blockManager(dbName);
        File file = new File(dbName);
        File[] files = file.listFiles();
        for (File f : files) {
            if (f.isDirectory()) {
                loadTableAllBlock(ret, dbName + "\\" + f.getName());
            }
        }
        return ret;
    }

    // 加载数据库中某一张表的所有block
    public static void loadTableAllBlock(blockManager ret, String tablePath) throws Exception {
        HashMap<String, Block> tableBlocks = new HashMap<String, Block>();
        File file = new File(tablePath);
        File[] files = file.listFiles();
        for (File f : files) {
            if (isDigit(f.getName())) {
                //这里的f.getName 带有.txt
                tableBlocks.put(f.getName(), (Block) tool.readObject(tablePath + "\\" + f.getName()));

            }
        }
        ret.blocks.put(file.getName(), tableBlocks);
    }

    // 判断该字符串是否以数字开头
    public static boolean isDigit(String temp) {
        char tempChar = temp.charAt(0);
        return 48 <= (int) tempChar && (int) tempChar <= 57;

    }

    public boolean isBlockIn(String name, String id) {
        return blocks.get(name).containsKey(id);
    }

    public void insert(String tableName, String path) throws Exception {
        Block b = (Block) tool.readObject(path);
        // TODO 这里暂时不考虑块的数量的上限
        blocks.get(tableName).put(b.id, b);
        cnt++;
    }

    // TODO 这是随机块替换函数, 暂时不用
    public void randomSave() {
        String tableName = "";
        int nowSize = -1;
        for (String name : blocks.keySet()) {
            if (blocks.get(name).size() > nowSize) {
                nowSize = blocks.get(name).size();
                tableName = name;
            }
        }
        HashMap<String, Block> t = blocks.get(tableName);
        int tSize = t.size();
        int rand = (int) (Math.random() * tSize);
        if (rand == tSize) {
            rand--;
        }
        int cntTemp = 0;
        for (String key : t.keySet()) {
            if (cntTemp == rand) {
                Block b = t.get(key);
                try {
                    tool.saveObject(b, b.path + "\\" + b.id + ".txt");
                    t.remove(key);
                    cnt--;
                } catch (Exception e) {
                    e.printStackTrace();
                    System.out.println("替换失败");
                }
            }
        }
    }

    // 保存blockManager中所有的block
    public void saveAll() throws Exception {
        for (String name : blocks.keySet()) {
            HashMap<String, Block> temp = blocks.get(name);
            for (Block tempBlock : temp.values()) {
                tool.saveObject(tempBlock, tempBlock.path + "\\" + tempBlock.id + ".txt");
            }
        }
    }

}
