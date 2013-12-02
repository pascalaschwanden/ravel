#ifndef STEPVIS_H
#define STEPVIS_H

#include "viswidget.h"
#include "colormap.h"
#include "event.h"

class StepVis : public VisWidget
{
    Q_OBJECT
public:
    StepVis(QWidget* parent = 0);
    ~StepVis();
    void setTrace(Trace * t);
    void processVis();
    //void resizeEvent(QResizeEvent * event);

    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void wheelEvent(QWheelEvent * event);
    void mouseDoubleClickEvent(QMouseEvent * event);

public slots:
    void setSteps(float start, float stop);
    void selectEvent(Event * event);

protected:
    void qtPaint(QPainter *painter);

private:
    bool mousePressed;
    int mousex;
    int mousey;
    int pressx;
    int pressy;
    int stepwidth;
    int processheight;
    bool showAggSteps;
    int maxStep;
    float startStep;
    float startProcess; // refers to order rather than process really
    float stepSpan;
    float processSpan;
    long long maxLateness;
    QMap<int, int> proc_to_order;
    QMap<int, int> order_to_proc;
    ColorMap * colormap;
};

#endif // STEPVIS_H
