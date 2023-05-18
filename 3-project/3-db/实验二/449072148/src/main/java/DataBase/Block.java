package DataBase;

import java.io.Serializable;
import java.util.ArrayList;

public class Block implements Serializable {
    public ArrayList<ArrayList<String>> block;
    public String path;
    // TODO 全文除了这里的id没带有.txt后缀外,其他所有地方出现了块文件的名称都带有.txt后缀
    public String id;
    public StringBuilder bitmap;
    public int limit;
    // TODO cnt字段表示此Block中有效的数据项数,由于删除打标机的缘故, block.size()无法表示有效的数据项数
    public int cnt;

    public Block(String path, String id, int limit) {
        block = new ArrayList<ArrayList<String>>();
        this.path = path;
        this.id = id;
        this.limit = limit;
        this.cnt = 0;
        bitmap = new StringBuilder();
        for (int i = 0; i < limit; i++) {
            bitmap.append("0");
        }
    }

    // 返回块id+".txt",以及该数据在此block的行数
    public String add(ArrayList<String> values) {
        if (block.size() < limit) {
            block.add(values);
            bitmap.setCharAt(block.size() - 1, '1');
            this.cnt++;
            return id + ".txt_" + (block.size() - 1);
        }
        for (int i = 0; i < limit; ++i) {
            if (bitmap.charAt(i) == '0') {
                block.set(i, values);
                bitmap.setCharAt(i, '1');
                this.cnt++;
                return id + ".txt_" + (i - 1);
            }
        }
        return id + ".txt_-1";
    }

    public void del(int index) {
        bitmap.setCharAt(index, '0');
        this.cnt--;
    }

    public void update(int index, ArrayList<String> values) {
        if (index >= limit) {
            return;
        }
        block.set(index, values);
    }

    public ArrayList<String> query(int column) {
        ArrayList<String> ret = new ArrayList<String>();
        for (ArrayList<String> i : block) {
            ret.add(i.get(column));
        }
        return ret;
    }

    public void printAll() {
        for (int i = 0; i < block.size(); i++) {
            printSingle(i);
        }
        //一个块打印完后输出一个换行符
        System.out.println();
    }

    // 根据下标精确输出
    public void printSingle(int cnt) {
        ArrayList<String> temp = block.get(cnt);
        if (bitmap.charAt(cnt) == '0') {
            return;
        }
        for (String s : temp) {
            String out = String.format("%10s", s);
            System.out.print(out);
        }
        System.out.println();
    }
}
