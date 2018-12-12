#ifndef USAGEWIDGET_H
#define USAGEWIDGET_H

#include <QLabel>
#include <QThread>

class UsageWidget : public QLabel {
    Q_OBJECT
public:
    explicit UsageWidget(QWidget *parent = nullptr);
    void setThreadsUsage(const int& threads);
    void setThreadsTotal(const int& threads);
    void setCpuUsage(const int& thisPercent);
    void setRamUsage(const qreal& thisGB);
    void setTotalRam(const qreal& totalRamGB);
private:
    void updateDisplayedText();

    int mThreadsUsage = 0;
    int mThreadsTotal = 0;
    int mCpuUsage = 0;
    qreal mRamUsage = 0.;
    qreal mTotalRam = 0.;
};

#endif // USAGEWIDGET_H