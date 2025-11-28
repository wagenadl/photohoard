// AutoCompletingLineEdit.cpp

#include "AutoCompletingLineEdit.h"

AutoCompletingLineEdit::AutoCompletingLineEdit(QWidget *parent):
  QLabel(parent) {
  allowmulti = false;
  allownonexist = false;
  cursorblink = false;
  startTimer(500); // for blinking cursor
}

AutoCompletingLineEdit::~AutoCompletingLineEdit() {
}

QStringList AutoCompletingLineEdit::optionsStartingWith(QString pfx) const {
  QStringList res;
  for (QString const &option: universe)
    if (option.lower().startsWith(pfx.lower()))
      res << option;
  return res;
}

QStringList AutoCompletingLineEdit::optionsContaining(QString ifx) const {
  QStringList res;
  for (QString const &option: universe)
    if (option.lower().contains(ifx.lower()))
      res << option;
  return res;
}

bool AutoCompletingLineEdit::exists(QString opt) const { 
  for (QString const &option: universe)
    if (option.lower() == opt.lower())
      return true;
  return false;
}

void AutoCompletingLineEdit::setUniverse(QStringList uni) {
  universe = uni;
  resuggest();
}

void AutoCompletingLineEdit::allowMultiple(bool b) {
  allowmulti = b;
  resuggest();
}

void AutoCompletingLineEdit::allowNonexistent(bool b) {
  allownonexist = b;
}

bool AutoCompletingLineEdit::acceptable() const {
  if (allowmulti) 
    return acceptable(parse());
  else
    return acceptable(state.usertext.trimmed());
}

bool AutoCompletingLineEdit::acceptable(QString s) {
  return allownonexistent ? true : exists(s);
}

bool AutoCompletingLineEdit::acceptable(QVariant const &v) const {
  if (v.metaType()==QMetaType::QString)
    return acceptable(v.toString());
  if (v.metaType()==QMetaType::QVariantList) {
    QVariantList const &lst(v.toList());
    if (lst.size()==3)
      return acceptable(lst[1]) && acceptable(lst[2]);
  }
  return false;
}

enum class TokType {
  STR,
  OPEN,
  CLOSE,
  AND,
  OR
};

static TokType toktype(QString s) {
  if (s=="|")
    return TokType::OR;
  else if (s=="&")
    return TokType::AND;
  else if (s=="(")
    return TokType::OPEN;
  else if (s==")")
    return TokType::CLOSE;
  else
    return TokType::STR;
}

static QVariant rparse(QStringList &bits) {
  if (bits.size()==0)
    return QVariant();
  QString bit = bits.takeLast();
  TokType typ = toktype(bit);
  QVariant v(bit);
  if (typ == TokType::CLOSE) 
    v = rparse(bits);
  else if (typ != TokType::STR)
    return QVariant();
  if (v.isNull())
    return v;
  if (bits.size()==0)
    return v;
  typ = toktype(bits.takeLast());
  if (typ == TokType::OPEN) {
    return v;
  } else if (typ == TokType::OR) {
    QVariant v1 = rparse(bits);
    return QVariantList{"|", v1, v};
  } else if (typ == TokType::"&") {
    QVariant v1 = rparse(bits);
    return QVariantList{"&", v1, v};
  } else {
    return QVariant();
  }
}
  

QVariant AutoCompletingLineEdit::parse() const {
  if (allowmulti) {
    /* The following would be so much easier in python:
         bits = re.split(r"([)(|&])", txt)
         bits = [bit.strip() for bit in bits]
         bits = [bit for bit in bits if bit!=""]
    */
    QStringList bits;
    QString bit;
    for (QChar c: state.usertext) {
      if (QString("()&|").contains(c)) {
        bit = bit.trimmed();
        if (bit != "")
          bits << bit;
        bits << QString(c);
        bit = "";
      } else {
        bit += c;
      }
    }
    bit = bit.trimmed();
    if (bit != "")
      bits << bit;
    return parse(bits);
  } else {
    return QVariant(state.usertext.trimmed());
  }
} 
  
void AutoCompletingLineEdit::paintEvent(QPaintEvent *) {
}

void AutoCompletingLineEdit::keyPressEvent(QKeyEvent *) {
}
