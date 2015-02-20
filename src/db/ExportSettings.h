// ExportSettings.h

#ifndef EXPORTSETTINGS_H

#define EXPORTSETTINGS_H

#include <QString>

class ExportSettings {
public:
  enum class FileFormat {
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
  enum class NamingScheme {
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
  uint maxdim;
  uint scalePercent;
  uint jpegQuality;
  NamingScheme namingScheme;
  QString destination;  
};

#endif
