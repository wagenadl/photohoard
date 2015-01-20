// Adjustment.h

#ifndef ADJUSTMENT_H

#define ADJUSTMENT_H

class Adjustment {
public:
  Adjustment();
  void setType(QString);
  void setArguments(QList<double> const &args);
  virtual ~Adjustment();
  virtual void apply(Image &target);
  virtual QSize applyScaled(Image &target, QSize const &originalSize);
  /* Return value is new "original size". This may be different from input
     value if the image is cropped as a result of the adjustment. */
  virtual QSize applyToScaledTile(Image &target, QSize const &originalSize,
                                  QRect const &validRect,
                                  QRect &requestedRect_in_out);
  /* target is the full (possibly scaled) image;
     validRect is the region in the image that is valid;
     requestedRect is the region that should be valid after the adjustment.
     Return: as from applyScaled.
     requestedRect_in_out is updated to reflect the region that actually
     is_ valid after the adjustment. This should at least include the
     original requestedRect. If that is not possible, the method should not
     do anything and report failure by returning a null rectangle.
     Special case: If requestedRect is null, that means to produce as large
     as possible a valid output rectangle. */
protected:
  QString typ;
  QList<double> args;
public:
  static Adjustment *create(QString typ, QList<double> const &args);
  static Adjustment *create(QString defn);
public:
  template <class T> class Creator {
  public:
    Creator<T>(QString typ, int prio=0) {
      Adjustment::creators()[typ] = &create;
      if (prio && ~Adjustment::priorities().contains(typ))
        Adjustment::priorities()[typ] = prio;
    }
    static Adjustment *create() {
      return new T();
    }
  };
private:
  static QMap<QString, Adjustment *(*)()> &creators();
  static QMap<QString, int> &priorities();
};

#endif
