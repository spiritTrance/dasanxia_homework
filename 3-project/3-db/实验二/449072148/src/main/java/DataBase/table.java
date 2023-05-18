package DataBase;

import javafx.util.Pair;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Scanner;

public class table {
    public static void CreateDB(String name) throws IOException {
        File file = new File(name);
        file.mkdirs();
    }

    public static void CreateTable(String DB) {

        Scanner scan = new Scanner(System.in);
        System.out.print("输入表格名称：");
        String name = scan.nextLine();
        System.out.println("输入表格属性名称及类型，以:分隔，每行仅包含一个属性.");
        System.out.println("请在第一行输入主键,主键请在类型后以_pk结尾, 最后一行以单独的exit结尾：\"");

        File table = new File(DB + "\\" + name);
        table.mkdir();
        ArrayList<Pair<String, String>> params = new ArrayList<Pair<String, String>>();
        int size = 0;
        do {
            String line = scan.nextLine();
            if (line.equals("exit")) {
                break;
            }
            String[] column = line.split(":");
            params.add(new Pair<String, String>(column[0], column[1]));
            if (column[1].startsWith("I")) {
                size += 4;
            } else if (column[1].startsWith("F")) {
                size += 4;
            } else {
                String[] tmp = column[1].split("_");
                size += Integer.parseInt(tmp[1]);
            }
        } while (true);
        // 不需要像tableManager里插入__limit__因为后续用到此字段时从第一个block里取出即可
        // params.add(new Pair<String, String>("__limit__",String.valueOf((int)4096 / size)));
        main.tManager.insert(name, params);
        // 创建该数据库的第一个块
        Block firstBlock = new Block(DB + "\\" + name, "1", 4096 / size);

        HashMap<String, Block> temp = new HashMap<String, Block>();
        temp.put("1.txt", firstBlock);
        main.bManager.blocks.put(name, temp);

        // 为该表建立索引
        ref tableRef = new ref(DB + "\\" + name);
        main.rManager.allRef.put(name, tableRef);
    }

    public static void insert(String tablePath, String tableName, ArrayList<String> values) {
        HashMap<String, Block> tableBlocks = main.bManager.blocks.get(tableName);
        for (Block tempBlock : tableBlocks.values()) {
            if (tempBlock.cnt < tempBlock.limit) {
                String ans = tempBlock.add(values);
                ref tempRef = main.rManager.allRef.get(tableName);
                tempRef.insert(values.get(0), ans);
                return;
            }
        }
        int blockCnt = tableBlocks.size() + 1;
        Block newBlock = new Block(tablePath, Integer.toString(blockCnt), tableBlocks.get("1.txt").limit);
        // 插入数据
        String ans = newBlock.add(values);
        // 插入blockManager
        main.bManager.blocks.get(tableName).put(blockCnt + ".txt", newBlock);
        // 插入rManager
        ref tempRef = main.rManager.allRef.get(tableName);
        tempRef.insert(values.get(0), ans);
    }

    public static void insertAll(String tablePath, String tableName, ArrayList<String> values, int times) {
        for (int i = 0; i <= times; ++i) {
            insert(tablePath, tableName, values);
            ArrayList<String> tmp = new ArrayList<String>();
            tmp.add(String.valueOf(Integer.parseInt(values.get(0)) + 1));
            for (int j = 1; j < values.size(); ++j)
                tmp.add("");
            values = tmp;
            System.out.println(values.get(0));
        }
    }

    // 打印一个表中的所有内容
    public static void findAll(HashMap<String, Block> tableBlocks) {
        for (Block tempBlock : tableBlocks.values()) {
            tempBlock.printAll();
        }
    }

    // 打印一个表的属性头
    public static void printTableHeader(String tableName) {
        ArrayList<Pair<String, String>> prams = main.tManager.allTables.get(tableName);
        for (Pair<String, String> pram : prams) {
            String out = String.format("%10s", pram.getKey());
            System.out.print(out);
        }
        System.out.println();
    }

    // 根据主键查找一个对象并打印
    public static void findSingle(String DBName, String tableName, String[] Value) {
        String column = Value[0];
        String value = Value[1];
        int index = -1;
        ArrayList<Pair<String, String>> table = main.tManager.allTables.get(tableName);
        for (int i = 0; i < table.size(); ++i) {
            if (table.get(i).getKey().equals(column)) {
                index = i;
                break;
            }
        }
        if (index == 0 && main.rManager.getRef(tableName).r.containsKey(value)) {
            String location = main.rManager.getRef(tableName).r.get(value);
            // 第一个变量记录它在哪一个块,第二个变量记录它在这个块的哪一行
            String[] loc = location.split("_");
            Block temp = main.bManager.blocks.get(tableName).get(loc[0]);
            temp.printSingle(Integer.parseInt(loc[1]));
        } else if (index != -1) {
            HashMap<String, Block> tmp = main.bManager.blocks.get(tableName);
            int cnt = 0;
            boolean flag = false;
            for (Block i : tmp.values()) {
                for (ArrayList<String> line : i.block) {
                    if (line.get(index).equals(value)) {
                        i.printSingle(cnt);
                        flag = true;
                    }
                    cnt += 1;
                }
                cnt = 0;
            }
            if (!flag)
                System.out.println("查找记录不存在");
        } else {
            System.out.println("查找字段不存在");
        }
    }

    // 根据主键删除对象
    public static void del(String tableName, String pkValue) {
        if (main.rManager.getRef(tableName).r.containsKey(pkValue)) {
            String location = main.rManager.getRef(tableName).r.get(pkValue);
            // 第一个变量记录它在哪一个块,第二个变量记录它在这个块的哪一行
            String[] loc = location.split("_");
            Block temp = main.bManager.blocks.get(tableName).get(loc[0]);
            temp.del(Integer.parseInt(loc[1]));
            // 将该对象从索引文件中删除
            main.rManager.getRef(tableName).r.remove(pkValue);
        } else {
            System.out.println("该对象不存在,请检查主键是否输入正确!");
        }
    }

    public static void update(String tableName, String pkValue, ArrayList<String> values) {
        if (main.rManager.getRef(tableName).r.containsKey(pkValue)) {
            String location = main.rManager.getRef(tableName).r.get(pkValue);
            // 第一个变量记录它在哪一个块,第二个变量记录它在这个块的哪一行
            String[] loc = location.split("_");
            Block temp = main.bManager.blocks.get(tableName).get(loc[0]);
            temp.update(Integer.parseInt(loc[1]), values);
        } else {
            System.out.println("该对象不存在,请检查主键是否输入正确!");
        }
    }

}
