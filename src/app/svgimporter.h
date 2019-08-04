#ifndef SVGIMPORTER_H
#define SVGIMPORTER_H
#include <QDebug>
#include <QtXml/QDomDocument>
#include <QtMath>
#include "paintsettingsapplier.h"
class VectorPath;
class Canvas;
class ContainerBox;
class BoundingBox;
enum CtrlsMode : short;

class TextSvgAttributes {
public:
    TextSvgAttributes() {}

    void setFontFamily(const QString &family);
    void setFontSize(const int size);
    void setFontStyle(const QFont::Style &style);
    void setFontWeight(const int weight);

    void setFontAlignment(const Qt::Alignment &alignment);

    const QFont &getFont() const { return mFont; }
private:
    Qt::Alignment mAlignment = Qt::AlignLeft;
    QFont mFont;
};

struct SvgGradient {
    Gradient* fGradient;
    qreal fX1;
    qreal fY1;
    qreal fX2;
    qreal fY2;
    QMatrix fTrans;
};

class FillSvgAttributes {
public:
    FillSvgAttributes() {}

    void setColor(const QColor &val);

    void setColorOpacity(const qreal opacity);

    void setPaintType(const PaintType &type);

    void setGradient(const SvgGradient& gradient);

    const QColor &getColor() const;
    PaintType getPaintType() const;
    Gradient *getGradient() const;

    void apply(BoundingBox * const box) const;
    void apply(BoundingBox * const box,
               const PaintSetting::Target& target) const;
protected:
    qreal mOpacity = 1;
    QColor mColor;
    PaintType mPaintType = FLATPAINT;//NOPAINT;
    Gradient *mGradient = nullptr;
    QPointF mGradientP1;
    QPointF mGradientP2;
};

class StrokeSvgAttributes : public FillSvgAttributes {
public:
    StrokeSvgAttributes() {}

    qreal getLineWidth() const;
    SkPaint::Cap getCapStyle() const;
    SkPaint::Join getJoinStyle() const;
    QPainter::CompositionMode getOutlineCompositionMode() const;

    void setLineWidth(const qreal val);

    void setCapStyle(const SkPaint::Cap capStyle);
    void setJoinStyle(const SkPaint::Join joinStyle);

    void setOutlineCompositionMode(const QPainter::CompositionMode compMode);

    void apply(BoundingBox *box, const qreal scale) const;
protected:
    SkPaint::Cap mCapStyle = SkPaint::kRound_Cap;
    SkPaint::Join mJoinStyle = SkPaint::kRound_Join;
    QPainter::CompositionMode mOutlineCompositionMode =
            QPainter::CompositionMode_Source;
    qreal mLineWidth = 0;
};

class BoxSvgAttributes {
public:
    BoxSvgAttributes() {}

    virtual ~BoxSvgAttributes() {}

    void setParent(const BoxSvgAttributes &parent);

    const Qt::FillRule &getFillRule() const;
    const QMatrix &getRelTransform() const;
    const FillSvgAttributes &getFillAttributes() const;
    const StrokeSvgAttributes &getStrokeAttributes() const;
    const TextSvgAttributes &getTextAttributes() const;

    void loadBoundingBoxAttributes(const QDomElement &element);

    bool hasTransform() const;

    void apply(BoundingBox *box) const;
    void setFillAttribute(const QString &value);
    void setStrokeAttribute(const QString &value);
protected:
    void decomposeTransformMatrix();

    Qt::FillRule mFillRule = Qt::OddEvenFill;

    qreal mDx = 0;
    qreal mDy = 0;
    qreal mScaleX = 1;
    qreal mScaleY = 1;
    qreal mShearX = 0;
    qreal mShearY = 0;
    qreal mRot = 0;

    qreal mOpacity = 100;

    QMatrix mRelTransform;

    QString mId;

    FillSvgAttributes mFillAttributes;
    StrokeSvgAttributes mStrokeAttributes;
    TextSvgAttributes mTextAttributes;
};

struct NormalNodeData;
class SvgNormalNode {
public:
    SvgNormalNode(const QPointF &p1);

    void setC0(const QPointF &c0);
    void setP1(const QPointF& p1) {
        mP1 = p1;
    }
    void setC2(const QPointF &c2);

    CtrlsMode getCtrlsMode() const;
    QPointF c0() const;
    QPointF p1() const;
    QPointF c2() const;

    bool getC0Enabled() const;
    bool getC2Enabled() const;

    void applyTransfromation(const QMatrix &transformation);

    NormalNodeData toNormalNodeData() const;
private:
    void guessCtrlsMode();

    bool mC0Enabled = false;
    bool mC2Enabled = false;
    CtrlsMode mCtrlsMode = CTRLS_CORNER;
    QPointF mC0;
    QPointF mP1;
    QPointF mC2;
};

class PathAnimator;
class VectorPathAnimator;
class SmartPathAnimator;
class SvgSeparatePath {
public:
    SvgSeparatePath() {}

    void apply(SmartPathAnimator * const path);

    void closePath();

    void moveTo(const QPointF &e);
    void lineTo(const QPointF &e);
    void cubicTo(const QPointF &c1, const QPointF &c2, const QPointF &e);
    void quadTo(const QPointF &c, const QPointF &e);
    void pathArc(qreal rX, qreal rY,
                 const qreal xAxisRotation,
                 const int largeArcFlag, const int sweepFlag,
                 const qreal x, const qreal y,
                 const qreal curX, const qreal curY);

    void applyTransfromation(const QMatrix &transformation);
private:
    void pathArcSegment(const qreal xc, const qreal yc,
                        const qreal th0, const qreal th1,
                        const qreal rx, const qreal ry,
                        const qreal xAxisRotation);

    void addPoint(const SvgNormalNode &point);

    bool mClosedPath = false;
    SvgNormalNode * mLastPoint = nullptr;
    QList<SvgNormalNode> mPoints;
};

class SmartVectorPath;
class VectorPathSvgAttributes : public BoxSvgAttributes {
public:
    VectorPathSvgAttributes() {}

    SvgSeparatePath *newSeparatePath() {
        auto lastPath = std::make_shared<SvgSeparatePath>();
        mSvgSeparatePaths << lastPath;
        return lastPath.get();
    }
    void apply(SmartVectorPath * const path);
protected:
    QList<stdsptr<SvgSeparatePath>> mSvgSeparatePaths;
};


extern void loadElement(const QDomElement &element, ContainerBox *parentGroup,
                        const BoxSvgAttributes &parentGroupAttributes);
extern qsptr<BoundingBox> loadSVGFile(const QString &filename);
/*
#include <QStringRef>
#include <QPainterPath>
#include <QXmlStreamAttributes>

class BoxesGroup;

class VectorPath;

extern QVector<qreal> parsePercentageList(const QChar *&str);
extern bool resolveColor(const QStringRef &colorStr, QColor &color, QSvgHandler *handler);
extern bool parsePathDataFast(const QStringRef &dataStr, VectorPath *path);
extern VectorPath *createPathNode(BoxesGroup *parent,
                                const QXmlStreamAttributes &attributes);
extern QColor parseColor(const QSvgAttributes &attributes,
                       QSvgHandler *handler);
extern bool constructColor(const QStringRef &colorStr, const QStringRef &opacity,
                           QColor &color, QSvgHandler *handler);
*/
#endif // SVGIMPORTER_H