// ImportSelectorDialog.cpp

#include "ImportSelectorDialog.h"
#include "ui_ImportSelectorDialog.h"
#include "PDebug.h"

ImportSelectorDialog::ImportSelectorDialog(QStringList choices,
                                           QWidget *parent):
  QDialog(parent), choices(choices) {
  ui = new Ui_ImportSelectorDialog;
  ui->setupUi(this);
  for (auto s: choices)
    ui->options->addItem(s);
  ui->options->setCurrentRow(0);
  choice_ = "";
}

ImportSelectorDialog::~ImportSelectorDialog() {
}

void ImportSelectorDialog::select() {
  int idx = ui->options->currentRow();
  if (idx>=0 && idx<choices.size())
    choice_ = choices[idx];
  else
    choice_ = "";
  pDebug() << "choice is" << choice();
  accept();
}

void ImportSelectorDialog::cancel() {
  choice_ = "";
  reject();
}

QString ImportSelectorDialog::choose(QStringList choices) {
  auto *dlg = new ImportSelectorDialog(choices);
  QString c = dlg->exec() ? dlg->choice() : QString("");
  delete dlg;
  return c;
}
