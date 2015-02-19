// ExportSettings.cpp

#include "ExportSettings.h"

ExportSettings::ExportSettings() {
  fileFormat = FileFormat::JPEG;
  resolutionMode = ResolutionMode::Full;
  maxdim = 1000;
  scalePercent = 50;
  jpegQuality = 90;
  namingScheme = NamingScheme::Original;
  destination = "/tmp";
}

QString ExportSettings::extension() const {
  switch (fileFormat) {
  case FileFormat::JPEG:
    return "jpg";
  case FileFormat::PNG:
    return "png";
  case FileFormat::TIFF:
    return "tif";
  }
  return QString();
}
