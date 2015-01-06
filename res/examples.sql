-- Select all versions in a given year
update versions set selected=1 where id in (select versions.id from versions inner join photos on versions.photo==photos.id where photos.capturedate>='2008-01-01T00:00:00' and photos.capturedate<'2009-01-01T00:00:00');

-- Alternative (about 10% slower)
update versions set selected=1 where exists (select id from photos where id==versions.photo and photos.capturedate>='2008-01-01T00:00:00' and photos.capturedate<'2009-01-01T00:00:00');

-- Alternative (twice as slow)
update versions set selected=1 where exists (select versions.id from versions inner join photos on versions.photo==photos.id where photos.capturedate>='2008-01-01T00:00:00' and photos.capturedate<'2009-01-01T00:00:00');

-- Also about twice as slow, but should unselect everything else, except that it does not work at all:
update versions set selected = (select photos.capturedate>='2008-01-01T00:00:00' and photos.capturedate<'2009-01-01T00:00:00'  from versions inner join photos on versions.photo==photos.id);

-- With a separate table, defined as
create table selection (version integer, foreign key(version) references versions(id) on delete cascade on update cascade);
-- this statement is even faster:
insert into selection select versions.id from  versions inner join photos on versions.photo==photos.id where photos.capturedate>='2008-01-01T00:00:00' and photos.capturedate<'2009-01-01T00:00:00';
-- And this is very fast after that:
select count (*) from versions inner join selection on versions.id==selection.version;
-- And things like these are very fast:
select distinct photos.camera from versions inner join selection on versions.id==selection.version inner join photos on versions.photo==photos.id;
select photos.capturedate from versions inner join selection on versions.id==selection.version inner join photos on versions.photo==photos.id where photos.camera==3;

-- Surprisingly, this is not terribly fast:
delete from selection;

-- But this is faster when there is nothing to delete and otherwise similar:
delete from selection where version>0;

-- Perhaps I should have selection in memory?
-- Definitely yes:
attach database ':memory:' as M;
create table M.sel (
       version integer, 
       foreign key(version) references versions(id) 
               on delete cascade 
               on update cascade);

insert into M.sel 
       select versions.id from versions 
              inner join photos on versions.photo==photos.id 
              where photos.capturedate>='2008-01-01T00:00:00' 
                    and photos.capturedate<'2009-01-01T00:00:00';

select photos.capturedate from versions 
       inner join M.sel on versions.id==M.sel.version 
       inner join photos on versions.photo==photos.id 
       where photos.camera==3;

delete from M.sel;

-- Almost the same speed-up is achieved by setting:
pragma synchronous=off;

-- But is that dangerous?

-------------------------------------- CAUTION ----------------------------
-- A QSqlDatabase connection may only be used in the thread in which it
-- was created. I do not know why, or if that is really true for sqlite.
-- I cannot figure out whether it is safe to have multiple connections
-- to the same database in one application. Tricky!
