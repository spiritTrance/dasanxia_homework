-- create table
create table teacher(
    _TID char(5) not null,
    TNAME varchar(20),
    DEPT varchar(20),
    SALARY numeric(8,2),
    primary key(_TID)
);
create table student(
    SID varchar(5) not null,
    SNAME varchar(20),
    DEPT varchar(20),
    AGE integer(3),
    GENDER char(1),
    primary key(SID)
);
create table course(
    CID varchar(5) not null,
    CNAME varchar(20),
    DEPT varchar(20),
    CREDITS numeric(2,1),
    TEACHER varchar(20),
    primary key(CID)
);
create table SC(
    SID varchar(5) not null,
    CID varchar(5) not null,
    GRADE integer(2),
    primary key(SID, CID),
    foreign key(SID) references student(SID),
    foreign key(CID) references course(CID)
);
-- insert data
insert into teacher values 
    (14001, 'Katz', 'CS', 75000),
    (14002, 'Crick', 'Biology', 72000),
    (14003, 'Gold', 'Physics', 87000),
    (14004, 'Einstein', 'Physics', 95000),
    (14005, 'Kim', 'CS', 65000),
    (14006, 'Wu', 'Finance', 90000),
    (14007, 'Brandt', 'CS', 65000),
    (14008, 'Singh', 'Finance', 80000);

insert into student values 
    ('S1', 'Wangfeng', 'Physics', 20, 'M'),
    ('S2', 'Liu fang', 'Physics', 19, 'M'),
    ('S3', 'Chen yun', 'CS', 22, 'M'),
    ('S4', 'Wu kai', 'Finance', 19, 'M'),
    ('S5', 'Liu li', 'CS', 21, 'F'),
    ('S6', 'Dongqing', 'Finance', 18, 'F'),
    ('S7', 'Li', 'CS', 19, 'F'),
    ('S8', 'Chen', 'CS', 21, 'F'),
    ('S9', 'Zhang', 'Physics', 19, 'M'),
    ('S10','Yang', 'CS', 22, 'F'),
    ('S11','Wang', 'CS', 19, 'F');

insert into course values
    ('C1', 'DB', 'CS', 2.0, 'Li'),
    ('C2', 'maths', 'Mathematics', 2.0, 'Ma'),
    ('C3', 'chemistry', 'Chemistry', 2.5, 'Zhou'),
    ('C4', 'physics', 'Physics', 1.5, 'Shi'),
    ('C5', 'OS', 'CS', 2.0, 'Wen'),
    ('C6', 'Database', 'CS', 2.0, 'Katz'),
    ('C7', 'Algorithm', 'CS', 2.5, 'Gold'),
    ('C8', 'Java', 'CS', 1.5, 'Einstein'),
    ('C9', 'Marketing', 'Finance', 2.0, 'Wu');

insert into SC values
    ('S1', 'C1', 70),
    ('S1', 'C3', 81),
    ('S2', 'C4', 92),
    ('S2', 'C2', 85),
    ('S3', 'C1', 65),
    ('S3', 'C5', 57),
    ('S4', 'C1', 87),
    ('S5', 'C4', 83);

-- 2)在student表中，为姓名为’Zhang’且系信息错填为’Physics’的同学修改信息，将其系信息修改为’CS’;
update student SET DEPT = 'CS' where SNAME = 'Zhang' and DEPT = 'Physics';
select * from student;

-- 3)删除teacher表中，属于Finance学院的教师信息;
delete from teacher where DEPT = 'Finance';
-- 4)在teacher表中，为工资低于或等于70000的教师增长10%的工资，为工资高于70000的教师增长5%的工资。
update teacher
set SALARY=CASE
               WHEN SALARY<=70000 THEN SALARY*1.1
               ELSE SALARY*1.05
           END;
select * from teacher
-- 2.基本数据查询
-- 1）基于teacher表，找出“物理系Physics和生物系Biology”所有教师的名字和工资；
select TNAME, SALARY from teacher where DEPT = 'Physics' or DEPT = 'Biology';
-- 2）基于teacher表，列出教师所在的各个系名，要求每个系名仅出现一次；
select distinct DEPT from teacher;
-- 3）基于teacher表，希望查看“若将每位教师的工资提高20%后”各位教师的姓名和工资是多少；
select TNAME, SALARY * 1.20 from teacher;
-- 4）基于表student、SC和course，查看到计算机系CS的每位学生所选课程的所有信息，包括学生姓名、所在系、课程名称、课程学分的情况。
select student.SNAME, student.DEPT, course.CNAME, course.CREDITS
from student, SC, course
where student.SID = SC.SID and SC.CID = course.CID;

-- 3. 复杂数据查询
-- 1）查询全体学生的姓名、年龄；
select SNAME, AGE from student;
-- 2）查询所有选修过课的学生的学号；
select distinct SID from SC;
-- 3）查询考试成绩低于60分的学生的学号；
select distinct SID from SC where GRADE < 60;
-- 4）查询年龄在20至23之间的学生姓名、性别和年龄；
select SNAME, GENDER, AGE from student where AGE >= 20 and AGE <= 23;
-- 5）查询所有姓liu的学生的学号、姓名和年龄；
select SID, SNAME, AGE from student where SNAME like 'Liu%'
-- 6) 查询学习C1课程的学生最高分数；
select max(GRADE) from SC where CID = 'C1' 
-- 7) 查询各个课程号与相应的选课人数；
select CID, count(CID) from SC group by CID
-- 8) 查询选修C3课程的学生的姓名；
select SNAME from student, SC where student.SID = SC.SID and SC.CID = 'C3'
-- 9) 查询每一门课程的平均成绩。
select course.CNAME, avg(SC.GRADE)
from course, SC
where course.CID = SC.CID
group by course.CID


-- 4、学生自主上机实验内容（选做）（未获满分时，可酌情加分）
-- 在必做题的数据库中完成以下要求：
-- 学生只能选择自己学院开设的课程。发现CS学院有的同学选择了其他学院开设的课程。在SC表中删除这些错选的记录。
create trigger course_choose_valid_check after insert on SC
    referencing new row as nrow
    for each row
    when(
        1 = 1
    )
    begin
        rollback
    end;
-- drop trigger course_choose_valid_check;