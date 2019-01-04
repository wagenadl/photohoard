// Adjuster.cpp

#include "Adjuster.h"
#include <math.h>
#include "AdjusterXYZ.h"
#include "AdjusterIPT.h"
#include "AdjusterGeometry.h"
#include "AdjusterEqualize.h"
#include "AdjusterUMask.h"
#include "PDebug.h"
#include "Geometry.h"

Adjuster::Adjuster(QObject *parent): QObject(parent) {
  caching = true;
  keeporiginal = true;
  maxthreads = 1;
}

Adjuster::~Adjuster() {
}

void Adjuster::setMaxThreads(int n) {
  maxthreads = n;
}

void Adjuster::clear() {
  stages.clear();
}

bool Adjuster::isEmpty() const {
  return stages.isEmpty();
}

void Adjuster::setOriginal(Image16 const &image) {
  clear();
  stages << AdjusterTile(image);
}

void Adjuster::setReduced(Image16 const &image, PSize originalSize) {
  clear();
  stages << AdjusterTile(image, originalSize);
}


Image16 Adjuster::retrieveFull(Adjustments const &settings) {
  qDebug() << "RetrieveFull" << settings.getAll();
  resetCanceled();

  if (stages.isEmpty())
    return Image16();
  if (stages[0].stage != Stage_Original)
    return Image16();
  
  if (stages.last().settings == settings)
    return stages.last().image;

  if (!applySettings(settings))
    return Image16();

  return stages.last().image;
}

bool Adjuster::applySettings(Adjustments const &settings) {
  /* Order of stages here must match enum Stage */
  return applyFirstXYZ(settings)
    && applyEqualize(settings)
    && applyIPT(settings)
    && applyUMask(settings)
    && applyGeometry(settings);
}

Image16 Adjuster::retrieveReduced(Adjustments const &settings,
                                  PSize maxSize) {
  qDebug() << "RetrieveReduced" << maxSize << settings.getAll();
  resetCanceled();

  if (stages.isEmpty() || !stages[0].roi.isEmpty() || stages[0].image.isNull())
    return Image16();

  applyNeedBasedScaling(settings, maxSize);

  if (stages.last().settings == settings)
    return stages.last().image;

  if (!applySettings(settings))
    return Image16();

  return stages.last().image;
}

void Adjuster::applyNeedBasedScaling(Adjustments const &settings,
                                     PSize maxSize) {
  PSize needed = neededScaledOriginalSize(settings, maxSize);
  int k = 0;
  while (k+1<stages.size()) {
    if (stages[k+1].stage>Stage_Reduced)
      break;
    if (!stages[k+1].image.size().isLargeEnoughFor(needed)
        || !stages[k+1].roi.isEmpty()) {
      dropFrom(k+1);
      break;
    }
    k++;
  }

  double fac = stages[k].image.size().scaleFactorToSnuglyFitIn(needed);
  if (fac<0.8) {
    // It's worth scaling
    // Should we reduce excessive scale stacks? Probably. Later.
    dropFrom(k+1);
    stages << AdjusterTile(stages[k].image.scaledToFitSnuglyIn(needed),
			   stages[k].osize);
    stages.last().stage = Stage_Reduced;
  }
}

void Adjuster::dropFrom(int k) {
  while (stages.size()>k)
    stages.removeLast();
}

Image16 Adjuster::retrieveROI(Adjustments const &, QRect) {
  qDebug() << "Adjuster::retrieveROI NYI";
  return Image16();
}

Image16 Adjuster::retrieveReducedROI(Adjustments const &,
                                     QRect, PSize) {
  qDebug() << "Adjuster::retrieveReducedROI NYI";
  return Image16();
}

void Adjuster::enableCaching(bool ec) {
  caching = ec;
}

void Adjuster::disableCaching() {
  caching = false;
}

void Adjuster::preserveOriginal(bool po) {
  keeporiginal = po;
}


bool Adjuster::ensureAlreadyGood(AdjusterStage const &adj, int iparent,
				 Adjustments const &final) {
  if (stages.size()>iparent+1) {
    Adjustments const &current = stages[iparent+1].settings;
    if (adj.isEquivalent(current, final)) 
      return true; // previous version of stage is fine
    else
      // everything that follows will be invalid
      while (stages.size()>iparent+1)
	stages.removeLast();
  }

  if (adj.isDefault(final))
    return true; // we don't need this stage
  else
    return false; // work to be done
}

int Adjuster::findParentStage(Stage s) const {
  for (int k=0; k<stages.size(); k++)
    if (stages[k].stage>=s)
      return k-1;
  return stages.size()-1;
}

bool Adjuster::applyGeometry(Adjustments const &final) {
  /* Here we apply rotate, perspective, and crop. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  int iparent = findParentStage(Stage_Geometry);
  if (iparent<0)
    return false;

  AdjusterGeometry adj(maxthreads);
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);

  return true;
}

bool Adjuster::applyEqualize(Adjustments const &final) {
  /* Here we apply clarity. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  int iparent = findParentStage(Stage_Equalize);
  if (iparent<0)
    return false;

  AdjusterEqualize adj(maxthreads);
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);

  return true;
}

bool Adjuster::applyUMask(Adjustments const &final) {
  /* Here we apply unsharp mask. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  int iparent = findParentStage(Stage_UMask);
  if (iparent<0)
    return false;

  AdjusterUMask adj(maxthreads);
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);

  return true;
}

bool Adjuster::applyFirstXYZ(Adjustments const &final) {
  /* Here we apply expose, blackXX, and soon whiteXX. */
  /* For now, I am ignoring the "caching" and "keeporiginal" flags.
   */
  int iparent = findParentStage(Stage_XYZ);
  if (iparent<0)
    return false;

  AdjusterXYZ adj(maxthreads);
  // we could store this somewhere to enable reuse of LUTs
  if (ensureAlreadyGood(adj, iparent, final))
    return true;
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);

  return true;
}

bool Adjuster::applyIPT(Adjustments const &final) {
  int iparent = findParentStage(Stage_IPT);
  if (iparent<0)
    return false;
  AdjusterIPT adj(maxthreads);
   if (ensureAlreadyGood(adj, iparent, final))
    return true;
  if (isCanceled())
    return false;
  stages << adj.apply(stages[iparent], final);
  return true;
}

void Adjuster::cancel() {
  canceled = true;
}

bool Adjuster::isCanceled() {
  if (canceled) {
    canceled = false;
    return true;
  } else {
    return false;
  }
}

void Adjuster::resetCanceled() {
  canceled = false;
}

PSize Adjuster::maxAvailableSize(Adjustments const &settings) const {
  if (stages.isEmpty())
    return PSize();
  else if (stages[0].osize.isEmpty())
    return PSize();
  return Geometry::scaledCroppedSize(stages[0].osize, settings,
                                     stages[0].image.size());
}


PSize Adjuster::neededScaledOriginalSize(Adjustments const &settings,
                                         PSize desired) const {
  if (stages.isEmpty())
    return PSize();
  return Geometry::neededScaledOriginalSize(stages[0].osize, settings, desired);
}
