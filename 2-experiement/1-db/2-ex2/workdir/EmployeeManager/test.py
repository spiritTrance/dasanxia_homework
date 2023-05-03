import psycopg2
conn=psycopg2.connect(database="db_2020_01",user="db_user2020_31",password="db_user@123", host="116.205.157.173",port=8000)
cur=conn.cursor()
print("Connected!")
try:
    cur.execute("CREATE TABLE test(id integer,name varchar,sex varchar);")
except:
    print("Table already exists!")
finally:
    cur.execute("INSERT INTO test(id,name,sex) VALUES(%s,%s,%s)",(1,'Aspirin','M'))
    cur.execute("INSERT INTO test(id,name,sex) VALUES(%s,%s,%s)",(2,'Taxol','F'))
    cur.execute("INSERT INTO test(id,name,sex) VALUES(%s,%s,%s)",(3,'Dixheral','M'))
    
    cur.execute('SELECT * FROM test')
    results=cur.fetchall()

    print(results)
    conn.commit()
    conn.close()