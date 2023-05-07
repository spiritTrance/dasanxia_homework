-- create table
    create table department(
        DEPTNAME varchar(20) not null,
        EID integer(19),
        primary key(DEPTNAME)
    );

    create table employee(
        EID integer(10) not null,
        ENAME varchar(10) not null,
        GENDER varchar(1) not null,
        AGE integer(3),
        JOB varchar(10),
        DEPTNAME varchar(20),
        SALARY numeric(20,2),
        RANKING varchar(1),
        primary key(EID),
        foreign key(DEPTNAME) references department(DEPTNAME) on delete cascade on update cascade
    );


    create table project(
        PROJECTNAME varchar(1024) not null,
        DEPTNAME varchar(20) not null,
        STARTTIME date,
        ENDTIME date,
        primary key(PROJECTNAME),
        foreign key(DEPTNAME) references department(DEPTNAME) on delete cascade on update cascade
    );

    create table EM_PRJ(
        EID integer(19) not null,
        PROJECTNAME varchar(1024) not null,
        unique(EID, PROJECTNAME),
        foreign key(EID) references employee(EID) on delete cascade on update cascade,
        foreign key(PROJECTNAME) references project(PROJECTNAME) on delete cascade on update cascade
    );
    -- add foreign key
    alter table department add constraint FK_EMPLOYEE foreign key(EID) REFERENCES employee(EID) on delete cascade on update cascade;

-- drop table
    -- drop table employee;
    -- drop table department;
    -- drop table EM_PRJ;
    -- drop table project;

-- test value
    --- truncate department CASCADE;
    --- truncate employee CASCADE;
    --- truncate project CASCADE;
    --- truncate EM_PRJ CASCADE;
    insert into department values
        ('advertise', null),
        ('research', null),
        ('personnel', null),
        ('manage', null),
        ('develop', null);

    insert into employee values
        (1, '小明', 'M', 16, '摸鱼', null, null, 'U'),
        (2, '黄日天', 'M', 19, '监督摸鱼', 'manage', 25000.55, 'U');
        (9, '黄乐天', 'M', 19, '监督摸鱼', 'manage', 25000.55, 'U');

    insert into project VALUES
        ('基于深度学习的摸鱼自动化方案研发','develop', '20230501', '20240501'),
        ('对于摸鱼需求的市场调研','research', '20230301', '20240801'),
        ('招募摸鱼人才','personnel', '20230301', '20240801');

    insert into EM_PRJ VALUES
        (1, '基于深度学习的摸鱼自动化方案研发'),
        (2, '招募摸鱼人才'),
        (9, '基于深度学习的摸鱼自动化方案研发');