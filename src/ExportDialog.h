// ExportDialog.h

#ifndef EXPORTDIALOG_H

#define EXPORTDIALOG_H

#include <QDialog>

class ExportDialog: public QDialog {
  Q_OBJECT;
public:
  class Settings {
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
    Settings();
  public:
    FileFormat fileFormat;
    ResolutionMode resolutionMode;
    int maxdim;
    int scalePercent;
    int jpegQuality;
    NamingScheme namingScheme;
    QString destination;  
  };
public:
  ExportDialog(QWidget *parent=0);
  virtual ~ExportDialog();
  void setup(Settings const &);
  Settings settings() const;
public slots:
  DialogCode exec();
private:
  class Ui_exportDialog *ui;
};

#endif
