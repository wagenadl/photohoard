// Filter.h

#ifndef FILTER_H

#define FILTER_H

#include <QSet>
#include <QStringList>
#include <QDate>

class Filter {
public:
  Filter();
  void reset();
  // COLLECTION
  void setCollection(QString);
  void unsetCollection();
  bool hasCollection() const { return hascollection; }
  QString collection() const { return collection_; }
  // COLOR LABELS
  void setColorLabels(QSet<int>);
  void unsetColorLabels();
  bool hasColorLabels() const { return hascolorlabels; }
  QSet<int> colorLabels() const { return colorlabels; }
  bool includesColorLabel(int) const;
  // STAR RATING
  void setStarRating(int min, int max);
  void unsetStarRating();
  bool hasStarRating() const { return hasstarrating; }
  int minStarRating() const { return minstars; }
  int maxStarRating() const { return maxstars; }
  // STATUS
  void setStatus(bool accepted, bool unset, bool rejected);
  void unsetStatus();
  bool hasStatus() const { return hasstatus; }
  bool statusAccepted() const { return statusaccepted; }
  bool statusRejected() const { return statusrejected; }
  bool statusUnset() const { return statusunset; }
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
  int count(class PhotoDB &) const;
  QString joinClause() const;
  QString whereClause(class PhotoDB &) const;
  QString collectionClause(class PhotoDB &) const;
  QString colorLabelClause() const;
  QString starRatingClause() const;
  QString statusClause() const;
  QString cameraClause(class PhotoDB &) const;
  QString dateRangeClause() const;
  QString fileLocationClause(class PhotoDB &) const;
  QString tagsClause(class PhotoDB &) const;
private:
  bool hascollection;
  QString collection_;
  bool hascolorlabels;
  QSet<int> colorlabels;
  bool hasstarrating;
  int minstars, maxstars;
  bool hasstatus;
  bool statusaccepted, statusunset, statusrejected;
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
