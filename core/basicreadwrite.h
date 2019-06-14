#ifndef BASICREADWRITE_H
#define BASICREADWRITE_H
#include <QString>
#include <QIODevice>
#include "castmacros.h"
struct qCubicSegment1D;
class SmartPath;
class BrushPolyline;
struct AutoTiledSurface;

class QReadSrc : public QIODevice {
    using QIODevice::write;
public:
    static QReadSrc * sCastQIODevice(QIODevice * const src) {
        return reinterpret_cast<QReadSrc*>(src);
    }
};

class QWriteSrc : public QIODevice {
    using QIODevice::read;
public:
    static QWriteSrc * sCastQIODevice(QIODevice * const src) {
        return reinterpret_cast<QWriteSrc*>(src);
    }
};

extern QString gReadString(QIODevice * src);
extern bool gReadBool(QIODevice * src);

extern bool gRead(QIODevice* src, QString& targetStr);
extern bool gWrite(QIODevice* dst, const QString& strToWrite);

extern bool gRead(QIODevice* src, bool& value);
extern bool gWrite(QIODevice* dst, const bool value);

extern bool gRead(QIODevice* src, qCubicSegment1D& value);
extern bool gWrite(QIODevice* dst, const qCubicSegment1D& value);

extern bool gRead(QIODevice* src, SmartPath& value);
extern bool gWrite(QIODevice* dst, const SmartPath& value);

extern bool gRead(QIODevice* src, BrushPolyline& value);
extern bool gWrite(QIODevice* dst, const BrushPolyline& value);
#endif // BASICREADWRITE_H
