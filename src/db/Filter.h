// Filter.h

#ifndef FILTER_H

#define FILTER_H

#include <QSet>
#include <QStringList>
#include <QDate>
#include "PhotoDB.h"

inline uint qHash(PhotoDB::ColorLabel cl) { return qHash(int(cl)); }

class Filter {
public:
  Filter(class SessionDB *db=0);
  /* FILTER - Construct an empty filter for us on a database
     The filtersettings are _not_ loaded from the db. 
   */
  void reset();
  bool isTrivial() const;
  void saveToDb() const;
  void loadFromDb();
  // COLLECTION
  void setCollection(QString);
  void unsetCollection();
  bool hasCollection() const { return hascollection; }
  QString collection() const { return collection_; }
  // COLOR LABELS
  void setColorLabels(QSet<PhotoDB::ColorLabel>);
  void unsetColorLabels();
  bool hasColorLabels() const { return hascolorlabels; }
  QSet<PhotoDB::ColorLabel> colorLabels() const { return colorlabels; }
  bool includesColorLabel(PhotoDB::ColorLabel) const;
  // STAR RATING
  void setStarRating(int min, int max);
  void unsetStarRating();
  bool hasStarRating() const { return hasstarrating; }
  int minStarRating() const { return minstars; }
  int maxStarRating() const { return maxstars; }
  // STATUS
  void setStatus(bool accepted, bool unset, bool rejected, bool newi);
  void unsetStatus();
  bool hasStatus() const { return hasstatus; }
  bool statusAccepted() const { return statusaccepted; }
  bool statusRejected() const { return statusrejected; }
  bool statusUnset() const { return statusunset; }
  bool statusNewImport() const { return statusnew; }
  // CAMERA
  void setCamera(QString make, QString model, QString lens);
  void unsetCamera();
  bool hasCamera() const { return hascamera; }
  QString cameraMake() const { return cameramake; }
  QString cameraModel() const { return cameramodel; }
  QString cameraLens() const { return cameralens; }
  // DATE RANGE
  void setDateRange(QDate start, QDate end);
  void unsetDateRange();
  bool hasDateRange() const { return hasdaterange; }
  QDate startDate() const { return startdate; }
  QDate endDate() const { return enddate; }
  // FILE LOCATION
  void setFileLocation(QString);
  void unsetFileLocation();
  bool hasFileLocation() const { return hasfilelocation; }
  QString fileLocation() const { return filelocation; }
  // TAGS
  void setTags(QStringList);
  void unsetTags();
  bool hasTags() const { return hastags; }
  QStringList tags() const { return tags_; }
public:
  /* count grabs its own DBReadLock */
  int count() const;
  /* you must hold a DBReadLock before calling any of the xxClause functions */
  QString joinClause() const;
  QString whereClause() const;
  QString collectionClause() const;
  QString colorLabelClause() const;
  QString starRatingClause() const;
  QString statusClause() const;
  QString cameraClause() const;
  QString dateRangeClause() const;
  QString fileLocationClause() const;
  QString tagsClause() const;
private:
  class SessionDB *db;
  bool hascollection;
  QString collection_;
  bool hascolorlabels;
  QSet<PhotoDB::ColorLabel> colorlabels;
  bool hasstarrating;
  int minstars, maxstars;
  bool hasstatus;
  bool statusaccepted, statusunset, statusrejected, statusnew;
  bool hascamera;
  QString cameramake, cameramodel, cameralens;
  bool hasdaterange;
  QDate startdate, enddate;
  bool hasfilelocation;
  QString filelocation;
  bool hastags;
  QStringList tags_;
};

#endif
