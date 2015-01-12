create table sizes (
       -- Maximum dimensions for cache objects
       maxdim integer );

create table memthresh (
       -- Threshold above which an object gets stored in a separate file
       bytes integer );

create table cache (
       id integer primary key,
       version integer,
       maxdim integer,
       width integer,
       height integer,
       outdated integer,
       dbno integer, -- 0 means file
       unique(version, maxdim) on conflict replace );

create table queue (
       version integer unique on conflict ignore );
       
create index cacheidx on cache(version);

insert into memthresh(bytes) values(100000);
insert into sizes(maxdim) values(128);
insert into sizes(maxdim) values(384);
insert into sizes(maxdim) values(1024);
