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
