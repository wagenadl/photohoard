create table sizes (
       -- Maximum dimensions for cache objects
       maxdim integer );

create table memthresh (
       -- Threshold above which an object gets stored in a separate file
       bytes integer );

create table cache (
       id integer primary key,
       version integer,
       width integer,
       height integer,
       maxdim integer,
       outdated integer,
       infile integer,
       bits blob,
       unique(version,width,height) );

create index cache_version on cache(version);       

create table queue (
       version integer unique on conflict ignore );
       
insert into memthresh(bytes) values(200000);
insert into sizes(maxdim) values(128);
insert into sizes(maxdim) values(384);
insert into sizes(maxdim) values(1024);
