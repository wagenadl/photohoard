// Undo.h

#ifndef UNDO_H

#define UNDO_H

/*
  create table undogroups (
    ugrp integer primary key,
    desc text,
    t datetime );

  create table undosteps (
    ustp integer primary key,
    ugrp integer references undogroups(ugrp),
    undo text );

  create table undorows (
    ustp integer references undosteps(ustp),
    row integer,
    orig );
  
*/

class Undo {
public:
  Undo(class Database const &db);
  int createGroup(QString description);
  int createStep(int ugrp, QString select_sql, QString undo_sql);
  void undoStep(int ustp);
  void undoGroup(int ugrp);
};

/* An example of a description could be "Set color label to yellow on
   143 images."
*/

/* Creating an undo step is done as follows:

     insert into undosteps (ugrp, undo) values (UGRP, UNDO_SQL);
     insert into undorows (ustp, row, orig) select UGRP, SELECT_SQL;

   For instance, SELECT_SQL could be:
     id, colorlabel from versions where id in (select version from selection)

   The corresponding UNDO_SQL could be

     update versions set colorlabel=@original where id

   which would be expanded at undo time into

     update versions set colorlabel=VALUE where id in
       (select row from undorows where ustp==STEP and orig==VALUE);

   Several of these statements would be run, once for each VALUE. The values
   would be found using

     select distinct orig from undorows where ustp==STEP;

   Note that some undo steps might not need values, such as

     delete from appliedtags where tag==TAG and version

   which would have to be expanded at undo time info

     delete from appliedtags where tag==TAG and version in
       (select row from undorows where ustp==STEP);
        
   It might be useful to allow additional magic in UNDO_SQL. For instance,
   I could imagine wanting to be able to restore a photo from the trash.
   This would be much more involved than regular undos. I imagine maintaining
   tables named DELETEDVERSIONS and DELETEDPHOTOS and magic commands
   @restoreversion without ORIG value, and @restorephoto with ORIG
   value pointing to the trash location (?). DELETEDAPPLIEDTAGS would be
   needed as well. And if the FOLDERS table gets cleaned up in any way,
   that makes things even more complicated.

   This undo mechanism certainly could be used for adjustments, but it might
   be attractive to have a dedicated system that avoids storing the entire
   adjustment string every time. This could be an argument for pulling the
   MODS column out of VERSIONS and creating a new ADJUSTMENTS table:

     create table adjustments (
       ky string,
       val,
       version integer references versions(id) );
     create index adjidx on adjustments(version);

   Intriguingly, this would mean that the value of a slider would no longer
   _have_ to be stored as a string. I like that.

   Note that the flexibility of this Undo system implies that the ROW column
   cannot be a reference to any particular foreign key. This does mean that
   it is possible that referred rows disappear from the target table. But
   that doesn't really hurt thanks to the semantics of "where KEY in (select
   ...)."

   Some undos might not even need rows, so perhaps the e.g. above should
   be stored as

     update versions set colorlabel=$orig where id in @rows

   to be more general and actually easier to read.
   
*/
#endif
