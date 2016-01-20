// Here.h

#ifndef HERE_H

#define HERE_H

#include <QString>

#define HERE QString("(%1:%2 in %3) ").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__)

#endif
