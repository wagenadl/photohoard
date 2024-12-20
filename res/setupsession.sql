pragma foreign_keys = on;

create table sinfo (
       id text unique,
       val );

create table paths (
       photodb text,
       cachedir text);
       
create table filtersettings (
       k string,
       v );

create table exportsettings (
       k string,
       v );

create table currentvsn (
       -- This table contains a single row.
       -- In v. 1.0, "current" would auto-null on deletion of a version.
       -- Now we have to take care of that ourselves.
       version integer );

create table starting ( 
       -- This table is empty except while building the LightTable.
       -- That way, we can avoid double crashes.
       s integer );

create table expanded (
       d0 date,
       scl int,
       unique(d0, scl) on conflict ignore);

create table expandedfolders (
       path string,
       unique(path) on conflict ignore);
       
insert into sinfo(id, val) values("sessiondb", "1.4");

insert into currentvsn values(null);
