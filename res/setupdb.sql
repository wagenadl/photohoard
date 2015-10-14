pragma foreign_keys = on;

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
       foreign key(parentfolder) references folders(id) 
               on delete cascade
               on update cascade );

create table foldertree (
       ancestor integer,
       descendant integer,
       foreign key(ancestor) references folders(id)
               on delete cascade on update cascade,
       foreign key(descendant) references folders(id)
               on delete cascade on update cascade 
       unique(ancestor, descendant) on conflict ignore );

create table excludedtrees (
       pathname text unique
                on conflict ignore );

create table tags (
       id integer primary key,
       tag text,
       parent integer,
       foreign key(parent) references tags(id)
               on delete cascade
               on update cascade);

create table cameras (
       id integer primary key,
       make text,
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
       -- Width and height as in the file, i.e., not corrected for orientation
       camera integer,
       lens integer,
       exposetime real,
       fnumber real,
       focallength real,
       distance real,
       iso real,
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
       orient integer,
       starrating integer default 0,
       colorlabel integer default 0,
       acceptreject integer default 0,
       foreign key(photo) references photos(id)
               on delete cascade
               on update cascade);

create table adjustments (
-- Table of adjustments to versions
       version integer,
       k text,
       v,
       unique(version, k) on conflict replace,
       foreign key(version) references versions(id)
               on delete cascade
               on update cascade );

create table undo (
-- Table of undo steps
       stepid integer primary key,
       version integer,
       item,
       oldvalue,
       newvalue,
       undone integer default 0,
       created date,
       foreign key(version) references versions(id)
               on delete cascade
               on update cascade );

create table appliedtags (
-- Table of tags applied to versions
       tag integer,
       version integer,
       foreign key(tag) references tags(id)
               on delete cascade
               on update cascade,
       foreign key(version) references versions(id)
               on delete cascade
               on update cascade,
       unique(tag, version) on conflict ignore );

create table defaulttags (
-- Table of tags applied to all new versions in a given folder
-- Intended for default collections
       tag integer,
       folder integer,
       foreign key(tag) references tags(id)
               on delete cascade
               on update cascade,
       foreign key(folder) references folders(id)
               on delete cascade
               on update cascade,
       unique(tag, folder) on conflict ignore );

create table folderstoscan (
       folder integer unique on conflict ignore,
       foreign key(folder) references folders(id) );

create table photostoscan (
       photo integer unique on conflict ignore,
       foreign key(photo) references photos(id) );

create table current (
       version integer references versions(id) 
               on delete set null );

create table expanded (
       d0 date,
       scl int,
       unique(d0, scl) on conflict ignore);

create table expandedfolders (
       path string,
       unique(path) on conflict ignore);

create table starting ( 
       -- This table is always empty except while building the LightTable
       -- That way, we can avoid double crashes.
       s integer );

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

create index if not exists photodateidx on photos(capturedate);
create index if not exists parentfolderidx on folders(parentfolder);
create index if not exists versionidx on versions(photo);
create index if not exists tagidx on tags(tag);
create index if not exists tagtreeidx on tags(parent);
create index if not exists undoidx on undo(version);

-- Not creating an index on photos(folder), becase that wouldn't optimize
-- the very common LIKE query, only the == query. (See PhotoDB::countInTree
-- and friends.) And the == query is already surprisingly fast (<1 us!?).
-- This suggests that perhaps the versionidx is not needed either. But that's
-- false, that index does improve speed on the admittedly irrelevant
--   select count(*) from versions where photo>10000;
-- Interestingly, the index does not substantially speed up things like
--   select count(*) from versions inner join photos 
--     on versions.photo==photos.id where camera==74;
-- And things like
--   select camera from versions inner join photos 
--     on versions.photo==photos.id where versions.id==231;
-- are blindingly fast (<1 us) even without the index.
-- I guess that things that might benefit would be
--   select id from versions where versions.photo==xxx
-- Yes, that goes from 6 ms without index to <1 with.

insert into current values(null);
