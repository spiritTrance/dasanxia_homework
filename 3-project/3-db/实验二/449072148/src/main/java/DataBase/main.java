package DataBase;

import javafx.util.Pair;

import java.io.File;
import java.util.ArrayList;
import java.util.Scanner;

public class main {
    // TODO 这里请改成自己存放总数据库的文件夹路径
    public static final String rootPath = "D:\\资料\\数据库\\代码";
    public static tableManager tManager;
    public static refManager rManager;
    public static String dbName = "";
    public static Scanner sc = new Scanner(System.in);
    public static String order1 = "", order2 = "", order3 = "";
    public static blockManager bManager;

    public static void getMenu1() {
        System.out.println("创建数据库请输入 1");
        System.out.println("查找数据库请输入 2");
        System.out.println("进入数据库请输入 3");
        System.out.println("删除数据库请输入 4");
        System.out.println("获取菜单请输入   5");
        System.out.println("退出程序请输入   6");
    }

    public static void getMenu2() {
        System.out.println("创建表请输入              1");
        System.out.println("查找此数据库下所有的表请输入 2");
        System.out.println("向某表中插入数据请输入      3");
        System.out.println("删除某表的数据请输入        4");
        System.out.println("修改某表的数据请输入        5");
        System.out.println("查找某表的数据请输入        6");
        System.out.println("获取菜单请输入             7");
        System.out.println("删除某张表请输入            8");
        System.out.println("退出此数据库请输入          9");
    }

    public static void main(String[] args) throws Exception {
        getMenu1();
        while (!order1.equals("6")) {
            order1 = sc.nextLine();
            if ("1".equals(order1)) {
                System.out.println("请输入数据库名称");
                String name = sc.nextLine();
                dbName = rootPath + "\\" + name;
                table.CreateDB(dbName);
                tManager = new tableManager(rootPath + "\\" + name);
                bManager = new blockManager(dbName);
                rManager = new refManager(dbName);
                func1();
            } else if ("2".equals(order1)) {
                ArrayList<String> allDB = new ArrayList<String>();
                File file = new File(rootPath);
                File[] files = file.listFiles();
                for (File f : files) {
                    if (f.isDirectory()) {
                        allDB.add(f.getName());
                    }
                }
                System.out.println("共有以下数据库");
                for (String s : allDB) {
                    System.out.println(s);
                }
            } else if ("3".equals(order1)) {
                System.out.println("请输入您要进入的数据库名称");
                String name = sc.nextLine();
                dbName = rootPath + "\\" + name;
                tManager = DataBase.tableManager.loadTables(dbName + "\\tableManager.txt");
                bManager = blockManager.loadDBAllBlock(dbName);
                rManager = refManager.loadAllRef(dbName);
                func1();
            } else if ("4".equals(order1)) {
                System.out.println("请输入你要删除的数据库名称");
                dbName = sc.nextLine();
                dbName = rootPath + "\\" + dbName;
                File file = new File(dbName);
                deleteDirectory(file);
            } else if ("5".equals(order1)) {
                getMenu1();
            } else if ("6".equals(order1)) {
                bManager.saveAll();
                break;
            } else {
                System.out.println("请输入正确的指令.");
                getMenu1();
            }
        }
        if (tManager != null) {
            tableManager.writeTables(tManager);
        }
        if (rManager != null) {
            refManager.saveAllRef(rManager);
        }
        if (bManager != null) {
            bManager.saveAll();
        }
        System.out.println("程序已退出,欢迎再次使用本数据库!");
    }

    public static void func1() {
        getMenu2();
        order2 = sc.nextLine();
        while (!"9".equals(order2)) {
            if ("1".equals(order2)) {
                // create table
                table.CreateTable(dbName);
            } else if ("2".equals(order2)) {
                ArrayList<String> allTable = new ArrayList<String>();
                File file = new File(dbName);
                File[] files = file.listFiles();
                for (File f : files) {
                    if (f.isDirectory()) {
                        allTable.add(f.getName());
                    }
                }
                System.out.println("共有以下表");
                for (String s : allTable) {
                    System.out.println(s);
                }
            } else if ("3".equals(order2)) {
                // 插入数据
                System.out.println("请输入你要操作的表名");
                String tableName = sc.nextLine();
                String tablePath = dbName + "\\" + tableName;
                ArrayList<String> values = new ArrayList<String>();
                ArrayList<Pair<String, String>> prams = tManager.allTables.get(tableName);
                boolean flag = false;
                System.out.println("<测试用：用户请输入false>");
                boolean test = Boolean.parseBoolean(sc.nextLine());
                // 找到主键,并保证传入后续函数中的第一个值即为主键
                for (int i = 0; i < prams.size() && !flag; i++) {
                    System.out.print(prams.get(i).getKey());
                    System.out.println("  " + prams.get(i).getValue());
                    if (i == 0) {
                        System.out.println("请输入主键:");
                        String pkValue = sc.nextLine();
                        if (rManager.getRef(tableName).r.containsKey(pkValue)) {
                            System.out.println("表中已存在该主键,此次插入操作失败!");
                            flag = true;
                        } else {
                            values.add(pkValue);
                        }
                    } else {
                        System.out.println("请输入该属性的值");
                        values.add(sc.nextLine());
                    }
                }
                if (!flag) {
                    if (test) table.insertAll(tablePath, tableName, values, (int) 1e6);
                    else table.insert(tablePath, tableName, values);
                }
            } else if ("4".equals(order2)) {
                // 删除数据
                System.out.println("请输入你要操作的表名");
                String tableName = sc.nextLine();
                System.out.println("请输入你要删除的对象的主键");
                String pkValue = sc.nextLine();
                table.del(tableName, pkValue);
            } else if ("5".equals(order2)) {
                // 修改数据
                System.out.println("请输入你要操作的表名");
                String tableName = sc.nextLine();
                ArrayList<String> values = new ArrayList<String>();
                ArrayList<Pair<String, String>> prams = tManager.allTables.get(tableName);
                boolean flag = false;
                String pkValue = "";
                // 找到主键,并保证传入后续函数中的第一个值即为主键
                for (int i = 0; i < prams.size() && !flag; i++) {
                    System.out.print(prams.get(i).getKey());
                    System.out.println("  " + prams.get(i).getValue());
                    if (i == 0) {
                        System.out.println("请输入您要修改的对象的主键:");
                        pkValue = sc.nextLine();
                        if (!rManager.getRef(tableName).r.containsKey(pkValue)) {
                            System.out.println("表中不存在该主键,此次修改操作失败!");
                            flag = true;
                        } else {
                            values.add(pkValue);
                        }
                    } else {
                        System.out.println("请输入该属性的值");
                        values.add(sc.nextLine());
                    }
                }
                if (!flag) {
                    table.update(tableName, pkValue, values);
                }
            } else if ("6".equals(order2)) {
                // 查找数据
                System.out.println("请输入你要查找的表名");
                String tableName = sc.nextLine();
                System.out.printf("本次查询共用时%d毫秒\n", find(tableName));
            } else if ("7".equals(order2)) {
                getMenu2();
            } else if ("8".equals(order2)) {
                System.out.println("请输入你要删除的表名");
                String tableName = sc.nextLine();
                File file = new File(dbName + "\\" + tableName);
                if (file.exists()) {
                    deleteDirectory(file);
                    tManager.allTables.remove(tableName);
                }
                System.out.println("成功删除此表!");
            } else {
                System.out.println("请输入正确的命令");
                getMenu2();
            }
            order2 = sc.nextLine();
        }
        getMenu1();
    }

    public static void deleteDirectory(File file) {
        if (!file.isFile()) {
            // 首先得到当前的路径
            String[] childFilePaths = file.list();
            for (String childFilePath : childFilePaths) {
                File childFile = new File(file.getAbsolutePath() + "/" + childFilePath);
                deleteDirectory(childFile);
            }
        }
        file.delete();
    }

    public static long find(String tableName) {
        System.out.println("请问您是想查询全部数据还是根据主键查询数据?");
        System.out.println("0 表示查询全部数据, 1表示按照指定字段查询");
        String or = sc.nextLine();
        if ("0".equals(or)) {
            table.printTableHeader(tableName);
            long startTime = System.currentTimeMillis();
            table.findAll(bManager.blocks.get(tableName));
            long endTime = System.currentTimeMillis();
            return endTime - startTime;
        } else if ("1".equals(or)) {
            System.out.println("请输入您想要查询的字段名和对应取值（用空格隔开）：");
            String[] Value = sc.nextLine().split(" ");
            table.printTableHeader(tableName);
            long startTime = System.currentTimeMillis();
            table.findSingle(dbName, tableName, Value);
            long endTime = System.currentTimeMillis();
            return endTime - startTime;
        } else {
            System.out.println("请输入正确的指令,本次查询无效");
        }
        return -1;
    }
}
