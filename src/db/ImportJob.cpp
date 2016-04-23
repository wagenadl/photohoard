// ImportJob.cpp

#include "ImportJob.h"
#include "Scanner.h"
#include "Collector.h"
#include "PDebug.h"
#include "SessionDB.h"
#include "Filter.h"
#include "Tags.h"

ImportJob::ImportJob(class SessionDB *db,
                     class Scanner *scanner,
                     QList<QUrl> const &sources,
                     QObject *parent): QObject(parent),
                                       db(db), scanner_(scanner),
                                       copyin(0), collector(0),
                                       sources_(sources) {
  op = Operation::Import; // GUI will change
  autodest = true;
  srcdisp = CopyIn::Leave;
  authorized_ = false;
  complete_ = false;
}

ImportJob::~ImportJob() {
  delete collector;
  delete copyin;
}

void ImportJob::countSources() {
  if (collector)
    return;
  
  collector = new Collector(this);
  connect(collector, SIGNAL(progress(int,int)),
          SIGNAL(countsUpdated(int,int)));
  connect(collector, SIGNAL(complete()), SLOT(markFinalSourceCount()));
  collector->collect(sources_);
}

void ImportJob::setAutoCollection() {
  Filter filter(db);
  filter.loadFromDb();
  if (filter.hasCollection()) {
    coll = filter.collection();
  } else {
    QStringList collections = Tags(db).collections();
    if (collections.isEmpty())
      coll = "";
    else
      coll = collections.first();
  }
  if (autodest)
    setAutoDestination();
}

void ImportJob::setOperation(Operation o) {
  op = o;
}

void ImportJob::setDestination(QString d) {
  dest = d;
  autodest = false;
}
 
void ImportJob::setMovieDestination(QString d) {
  moviedest = d;
}

void ImportJob::setNoMovieDestination() {
  moviedest = "";
}

void ImportJob::setSourceDisposition(CopyIn::SourceDisposition sd) {
  srcdisp = sd;
}

void ImportJob::setCollection(QString c) {
  coll = c;
}

void ImportJob::setAutoDestination() {
  autodest = true;

  QString root = "";
  int tagid = Tags(db).findCollection(coll);
  if (tagid>=0) {
    QSqlQuery q
      = db->constQuery("select pathname from folders"
                       " inner join defaulttags"
                       " on folders.id==defaulttags.folder"
                       " where tag==:a"
                       " order by length(pathname) limit 1",
                       tagid);
    if (q.next())
      root = q.value(0).toString();
  }

  if (root.isEmpty())
    root = db->simpleQuery("select pathname from folders"
                           " order by length(pathname) limit 1").toString();

  dest = CopyIn::autoDest(root);
}

int ImportJob::preliminarySourceCount() const {
  if (collector)
    return collector->preliminaryCount();
  else
    return 0;
}

bool ImportJob::hasSourceCount() const {
  return collector && collector->isComplete();
}

int ImportJob::sourceCount() {
  if (!collector)
    countSources();
  if (!hasSourceCount())
    collector->wait();
  return collector->imageFiles().size() + collector->movieFiles().size();
}  

void ImportJob::authorize() {
  authorized_ = true;
  switch (op) {
  case Operation::Import:
    if (hasSourceCount())
      startCopy();
    break;
  case Operation::Incorporate:
    CRASH("NYI");
    break;
  }
}

void ImportJob::markFinalSourceCount() {
  if (authorized_ && op==Operation::Import)
    startCopy();
}

void ImportJob::startCopy() {
  ASSERT(hasSourceCount());
  QStringList images = collector->imageFiles();
  QStringList movies = moviedest.isEmpty() ? QStringList()
    : collector->movieFiles();

  
  copyin = new CopyIn(this);
  connect(copyin, SIGNAL(completed(int,int)),
          SLOT(doneCopying(int,int)));
  connect(copyin, SIGNAL(progress(int)),
          this, SIGNAL(progress(int)));
  
  copyin->setDestination(dest);
  copyin->setMovieDestination(moviedest);

  copyin->setSources(images);
  copyin->setMovieSources(movies);

  copyin->setSourceDisposition(srcdisp);

  copyin->start();
}

void ImportJob::doneCopying(int,int) {
  qDebug() << "done copying";
  scanner_->addTree(dest, coll);
  emit complete("");
}

void ImportJob::cancel() {
  if (collector)
    collector->cancel();
  if (copyin)
    copyin->cancel();
  emit canceled();
}

QString ImportJob::commonRoot() const {
  return commonRoot(sources_);
}

QString ImportJob::commonRoot(QList<QUrl> const &urls) {
  QStringList parts;
  bool any = false;
  for (QUrl const &url: urls) {
    if (url.isLocalFile()) {
      QStringList p0 = url.path().split("/");
      if (!p0.isEmpty() && p0.last().contains("."))
        p0.takeLast();
      if (any) {
        while (p0.size()>parts.size())
          p0.takeLast();
        while (parts.size()>p0.size())
          parts.takeLast();
        while (!parts.isEmpty()) {
          if (p0.last()!=parts.last()) {
            p0.takeLast();
            parts.takeLast();
          }
        }
      } else {
        parts = p0;
        any = true;
      }
    }
  }
  return "/" + parts.join("/");
}
