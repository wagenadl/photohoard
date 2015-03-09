// ExportSettings.h

#ifndef EXPORTSETTINGS_H

#define EXPORTSETTINGS_H

#include <QString>

class ExportSettings {
public:
  enum class FileFormat { // Keep in order with ExportDialog ui!
    JPEG,
      PNG,
      TIFF,
      };
  enum class ResolutionMode {
    Full,
      LimitWidth,
      LimitHeight,
      LimitMaxDim,
      Scale,
      };
  enum class NamingScheme { // Keep in order with ExportDialog ui
    Original,
      DateTime,
      DateTimeDSC,
      };
public:
  ExportSettings();
  QString extension() const;
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
