#include "colorvaluerect.h"
#include "colorhelpers.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include "GUI/mainwindow.h"
#include "GUI/global.h"

ColorValueRect::ColorValueRect(const ColorProgram& program,
                               QWidget * const parent) :
    ColorWidget(parent), mGLProgram(program) {}

void ColorValueRect::paintGL() {
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mGLProgram.fID);
    if(mGLProgram.fHSVColorLoc >= 0) {
        glUniform3f(mGLProgram.fHSVColorLoc,
                    mHue, mSaturation, mValue);
    }
    if(mGLProgram.fRGBColorLoc >= 0) {
        float r = mHue;
        float g = mSaturation;
        float b = mValue;
        hsv_to_rgb_float(r, g, b);
        glUniform3f(mGLProgram.fRGBColorLoc, r, g, b);
    }
    if(mGLProgram.fHSLColorLoc >= 0) {
        float h = mHue;
        float s = mSaturation;
        float l = mValue;
        hsv_to_hsl(h, s, l);
        glUniform3f(mGLProgram.fHSLColorLoc, h, s, l);
    }
    glUniform1f(mGLProgram.fCurrentValueLoc, static_cast<float>(mVal));
    glUniform1f(mGLProgram.fHandleWidthLoc, 2.f/width());
    glUniform1i(mGLProgram.fLightHandleLoc,
                shouldValPointerBeLightHSV(mHue, mSaturation, mValue));
    if(mGLProgram.fMeshSizeLoc >= 0) {
        glUniform2f(mGLProgram.fMeshSizeLoc,
                    height()/(3.f*width()), 1.f/3);
    }
    glBindVertexArray(mPlainSquareVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ColorValueRect::mouseMoveEvent(QMouseEvent *e) {
    mouseInteraction(e->x());
    Document::sInstance->actionFinished();
}

void ColorValueRect::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::RightButton) return;
    emit editingStarted(value());
    mouseInteraction(e->x());
    Document::sInstance->actionFinished();
}

void ColorValueRect::mouseReleaseEvent(QMouseEvent *) {
    mHslSaturatonTmp = -1;
    emit editingFinished(value());
    Document::sInstance->actionFinished();
}

//void ColorValueRect::wheelEvent(QWheelEvent *e)
//{
//    if(e->delta() > 0) {
//        setValueAndEmitValueChanged(clamp(val + 0.01f, 0.f, 1.f));
//    }
//    else {
//        setValueAndEmitValueChanged(clamp(val - 0.01f, 0.f, 1.f));
//    }
//    MainWindow::getInstance()->callUpdateSchedulers();
//}

void ColorValueRect::mouseInteraction(const int x_t) {
    setValueAndEmitValueChanged(clamp(qreal(x_t)/width(), 0, 1));
}


void ColorValueRect::setDisplayedValue(const qreal val_t) {
    mVal = val_t;
    update();
}

void ColorValueRect::setValueAndEmitValueChanged(const qreal valT) {
    setDisplayedValue(valT);
    emit valueChanged(valT);
}