// Purge.cpp

#include "Purge.h"
#include "PhotoDB.h"
#include "AutoCache.h"
#include <QFileInfo>

Purge::Purge(PhotoDB *db, AutoCache *cache): db(db), cache(cache) {
  refresh();
}

void Purge::refresh() {
  vsnsWSiblings.clear();
  vsnsWoSiblings.clear();
  phsWOtherVsns.clear();
  phsWoOtherVsns.clear();
  phsThatCanBeDeld.clear();
  orphans.clear();
  orphsThatCanBeDeld.clear();
  
  db->query("create table M.rejects as"
            " select id as vsn, photo from versions"
	    " where acceptreject<0");
  db->query("create table M.allvsn as"
	    " select versions.id as vsn, acceptreject, versions.photo"
	    " from versions inner join photos on versions.photo==photos.id"
	    " where photos.id in (select photo from M.rejects)");
  db->query("create table M.keepphoto as"
	    " select photo from M.allvsn where acceptreject>=0 group by photo");

  {
    QSqlQuery q = db->constQuery("select vsn, photo, photo in M.keepphoto"
				 " from M.rejects");
    while (q.next()) {
      uint64_t vsn = q.value(0).toULongLong();
      uint64_t pht = q.value(1).toULongLong();
      bool keep = q.value(2).toBool();
      if (keep) {
	vsnsWSiblings.insert(vsn);
	phsWOtherVsns.insert(pht);
      } else {
	vsnsWoSiblings.insert(vsn);
	phsWoOtherVsns.insert(pht);
      }
    }
  }

  { QSqlQuery q = db->constQuery("select photo, filename, pathname"
				 " from M.rejects inner join photos"
				 " on M.rejects.photo==photos.id"
				 " left join folders"
				 " on photos.folder==folders.id"
				 " where photo not in"
				 " (select photo from M.keepphoto)"
				 " group by photo");
    while (q.next()) {
      uint64_t pht = q.value(0).toULongLong();
      QString filename = q.value(1).toString();
      QString foldername = q.value(2).toString();
      QFileInfo filei(foldername.isEmpty()
                      ? filename
                      : (foldername + "/" + filename));
      QFileInfo folderi(filei.absolutePath());
      bool candel = filei.isWritable() && folderi.isWritable();
      // Note that isWritable() returns user's ability to write, not owner's.
      if (candel)
	phsThatCanBeDeld[pht] = filei.absoluteFilePath();
    }
  }

  { QSqlQuery q = db->constQuery("select photos.id, filename, pathname"
                                 " from photos left join folders"
                                 " on photos.folder==folders.id"
                                 " where photos.id not in"
                                 " (select photo from versions)");
    while (q.next()) {
      uint64_t pht = q.value(0).toULongLong();
      orphans.insert(pht);
      QString filename = q.value(1).toString();
      QString foldername = q.value(2).toString();
      QFileInfo filei(foldername.isEmpty()
                      ? filename
                      : (foldername + "/" + filename));
      QFileInfo folderi(filei.absolutePath());
      bool candel = filei.isWritable() && folderi.isWritable();
      // Note that isWritable() returns user's ability to write, not owner's.
      if (candel)
	orphsThatCanBeDeld[pht] = filei.absoluteFilePath();
    }
  }

  db->query("drop table M.rejects");
  db->query("drop table M.allvsn");
  db->query("drop table M.keepphoto");
}
