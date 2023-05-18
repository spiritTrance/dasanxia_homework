package DataBase;


import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;

// TODO 序列化存放和加载的工具类
public class tool {
    public static void saveObject(Object object, String path) throws Exception {
        ObjectOutputStream out = null;
        FileOutputStream fout = null;
        try {
            fout = new FileOutputStream(path);
            out = new ObjectOutputStream(fout);
            out.writeObject(object);
        } finally {
            fout.close();
            out.close();
        }
    }

    public static Object readObject(String path) throws Exception {
        ObjectInputStream in = null;
        FileInputStream fin = null;
        try {
            fin = new FileInputStream(path);
            in = new ObjectInputStream(fin);
            return in.readObject();
        } finally {
            fin.close();
            in.close();
        }
    }

}
