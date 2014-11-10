create table info (
       id text,
       version text );

create table filetypes (
-- Table of file types with their most common extension
       id integer primary key,
       stdext text unique );

create table extensions (
-- Table of file extensions
       extension text unique,
       filetype integer,
       foreign key(filetype) references filetypes(id) 
               on delete cascade 
               on update cascade );

create table folders (
       id integer primary key,
       parentfolder integer,
       leafname text,
       pathname text unique,
       lastscan date,
       foreign key(parentfolder) references folder(id) 
               on delete cascade
               on update cascade );

create table foldertree (
-- for ease of referencing grandchild folders
       descendant integer,
       ancestor integer,
       foreign key(descendant) references folder(id)
               on delete cascade
               on update cascade,
       foreign key(ancestor) references folder(id)
               on delete cascade
               on update cascade );

create table cameras (
       id integer primary key,
       camera text );

create table lenses (
       id integer primary key,
       lens text );

create table photos (
-- Table of photographs as taken by a camera
       id integer primary key,
       folder integer,
       filename text,
       filetype integer,
       width integer,
       height integer,
       camera integer,
       lens integer,
       exposetime real,
       fnumber real,
       focallength real,
       distance real,
       iso real,
       orient integer,
       capturedate date,
       lastscan date,
       -- what else from exif?
       foreign key(folder) references folders(id)
               on delete cascade
               on update cascade,
       foreign key(filetype) references filetypes(id),
       foreign key(camera) references cameras(id),
       foreign key(lens) references lenses(id),
       unique(folder, filename) );

create table versions (
-- Table of derived versions of photographs
       id integer primary key,
       photo integer,
       ver integer,
       foreign key(photo) references photos(id)
               on delete cascade
               on update cascade,
       unique(photo, ver));

create table folderstoscan (
       folder integer unique on conflict ignore,
       foreign key(folder) references folders(id) );

create table photostoscan (
       photo integer unique on conflict ignore,
       foreign key(photo) references photos(id) );

-- ======================================================================

insert into filetypes(stdext) values ("jpeg");
insert into filetypes(stdext) values ("png");
insert into filetypes(stdext) values ("tiff");
insert into filetypes(stdext) values ("nef");
insert into filetypes(stdext) values ("cr2");

insert into extensions(filetype, extension)
       select id, stdext from filetypes;
insert into extensions(filetype, extension)
       select id, "jpg" from filetypes where stdext=="jpeg";
insert into extensions(filetype, extension)
       select id, "tif" from filetypes where stdext=="tiff";

insert into info values("PhotoDB", "1.0");
