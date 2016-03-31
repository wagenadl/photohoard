pragma foreign_keys = on;

create table sinfo (
       id text,
       version text );

create table photodb (
       fn text );
       
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
       -- This table contains a single row.
       -- Its value is always null except while building the LightTable.
       -- That way, we can avoid double crashes.
       s integer );

insert into sinfo values("PhotohoardSessionDB", "1.1");

insert into currentvsn values(null);

insert into starting values(null);
