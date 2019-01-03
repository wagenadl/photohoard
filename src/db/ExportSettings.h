// ExportSettings.h

#ifndef EXPORTSETTINGS_H

#define EXPORTSETTINGS_H

#include <QString>

class ExportSettings {
public:
  enum class FileFormat { // Keep in order with ExportDialog ui!
    JPEG=0,
      PNG,
      TIFF,
      };
  enum class ResolutionMode {
    Full=0,
      LimitWidth,
      LimitHeight,
      LimitMaxDim,
      Scale,
      };
  enum class NamingScheme { // Keep in order with ExportDialog ui
    Original=0,
      DateTime,
      DateTimeDSC,
      };
public:
  ExportSettings();
  QString extension() const;
  bool isValid() const;
  void loadFromDB(class PhotoDB const *db);
  void saveToDB(class PhotoDB *db);
  QString exportFilename(class PhotoDB *db, quint64 vsn) const;
public:
  FileFormat fileFormat;
  ResolutionMode resolutionMode;
  int maxdim;
  int scalePercent;
  int jpegQuality;
  NamingScheme namingScheme;
  QString destination;  
};

#endif
