// Extensions.h

#ifndef EXTENSIONS_H

#define EXTENSIONS_H

#include <QString>
#include <QSet>

namespace Extensions {
  QSet<QString> const &imageExtensions();
  QSet<QString> const &movieExtensions();
};

#endif
