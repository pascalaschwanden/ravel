#include "traditionalvis.h"
#include <iostream>
#include <QFontMetrics>

TraditionalVis::TraditionalVis(QWidget * parent) : TimelineVis(parent = parent)
{
    // Set painting variables
    stepToTime = new QVector<TimePair *>();
}

TraditionalVis::~TraditionalVis()
{
    for (QVector<TimePair *>::Iterator itr = stepToTime->begin(); itr != stepToTime->end(); itr++) {
        delete *itr;
        *itr = NULL;
    }
    delete stepToTime;
}


void TraditionalVis::setTrace(Trace * t)
{
    VisWidget::setTrace(t);

    // Initial conditions
    startStep = 0;
    int stopStep = startStep + initStepSpan;
    startProcess = 0;
    processSpan = trace->num_processes;

    // Determine time information
    minTime = ULLONG_MAX;
    maxTime = 0;
    startTime = ULLONG_MAX;
    unsigned long long stopTime = 0;
    maxStep = trace->global_max_step;
    for (QList<Partition*>::Iterator part = trace->partitions->begin(); part != trace->partitions->end(); ++part)
    {
        for (QMap<int, QList<Event *> *>::Iterator event_list = (*part)->events->begin(); event_list != (*part)->events->end(); ++event_list) {
            for (QList<Event *>::Iterator evt = (event_list.value())->begin(); evt != (event_list.value())->end(); ++evt) {
                if ((*evt)->exit > maxTime)
                    maxTime = (*evt)->exit;
                if ((*evt)->enter < minTime)
                    minTime = (*evt)->enter;
                if ((*evt)->step >= boundStep(startStep) && (*evt)->enter < startTime)
                    startTime = (*evt)->enter;
                if ((*evt)->step <= boundStep(stopStep) && (*evt)->exit > stopTime)
                    stopTime = (*evt)->exit;
            }
        }
    }
    timeSpan = stopTime - startTime;
    stepSpan = stopStep - startStep;

    for (QVector<TimePair *>::Iterator itr = stepToTime->begin(); itr != stepToTime->end(); itr++) {
        delete *itr;
        *itr = NULL;
    }
    delete stepToTime;

    stepToTime = new QVector<TimePair *>();
    for (int i = 0; i < maxStep/2 + 1; i++)
        stepToTime->insert(i, new TimePair(ULLONG_MAX, 0));
    int step;
    for (QList<Partition*>::Iterator part = trace->partitions->begin(); part != trace->partitions->end(); ++part)
    {
        for (QMap<int, QList<Event *> *>::Iterator event_list = (*part)->events->begin(); event_list != (*part)->events->end(); ++event_list) {
            for (QList<Event *>::Iterator evt = (event_list.value())->begin(); evt != (event_list.value())->end(); ++evt) {
                if ((*evt)->step < 0)
                    continue;
                step = (*evt)->step / 2;
                if ((*stepToTime)[step]->start > (*evt)->enter)
                    (*stepToTime)[step]->start = (*evt)->enter;
                if ((*stepToTime)[step]->stop < (*evt)->exit)
                    (*stepToTime)[step]->stop = (*evt)->exit;
            }
        }
    }

}

void TraditionalVis::mouseMoveEvent(QMouseEvent * event)
{
    if (!visProcessed)
        return;

    if (mousePressed) {
        int diffx = mousex - event->x();
        int diffy = mousey - event->y();
        //startTime += rect().width() * diffx / 1.0 / timeSpan;
        startTime += timeSpan / 1.0 / rect().width() * diffx;
        startProcess += diffy / 1.0 / processheight;

        if (startTime < minTime)
            startTime = minTime;
        if (startTime > maxTime)
            startTime = maxTime;

        if (startProcess + processSpan > trace->num_processes + 1)
            startProcess = trace->num_processes - processSpan + 1;
        if (startProcess < 0)
            startProcess = 0;


        mousex = event->x();
        mousey = event->y();
        repaint();
    }
    else // potential hover
    {
        mousex = event->x();
        mousey = event->y();
        if (hover_event == NULL || !drawnEvents[hover_event].contains(mousex, mousey))
        {
            hover_event = NULL;
            for (QMap<Event *, QRect>::Iterator evt = drawnEvents.begin(); evt != drawnEvents.end(); ++evt)
                if (evt.value().contains(mousex, mousey))
                    hover_event = evt.key();

            repaint();
        }
    }

    if (mousePressed) {
        changeSource = true;
        emit stepsChanged(startStep, startStep + stepSpan); // Calculated during painting
    }
}

void TraditionalVis::wheelEvent(QWheelEvent * event)
{
    if (!visProcessed)
        return;

    float scale = 1;
    int clicks = event->delta() / 8 / 15;
    scale = 1 + clicks * 0.05;
    if (Qt::MetaModifier && event->modifiers()) {
        // Vertical
        float avgProc = startProcess + processSpan / 2.0;
        processSpan *= scale;
        startProcess = avgProc - processSpan / 2.0;
    } else {
        // Horizontal
        float middleTime = startTime + timeSpan / 2.0;
        timeSpan *= scale;
        startTime = middleTime - timeSpan / 2.0;
    }
    repaint();
    changeSource = true;
    emit stepsChanged(startStep, startStep + stepSpan); // Calculated during painting
}

void TraditionalVis::setSteps(float start, float stop)
{
    if (!visProcessed)
        return;

    if (changeSource) {
        changeSource = false;
        return;
    }
    startTime = (*stepToTime)[std::max(boundStep(start)/2, 0)]->start;
    timeSpan = (*stepToTime)[std::min(boundStep(stop)/2,  maxStep/2)]->stop - startTime;

    if (!closed)
        repaint();
}

void TraditionalVis::qtPaint(QPainter *painter)
{
    if(!visProcessed)
        return;

    if ((rect().height() - timescaleHeight) / processSpan >= 3)
        paintEvents(painter);

    drawTimescale(painter, startTime, timeSpan);
    drawHover(painter);
}

void TraditionalVis::paintEvents(QPainter *painter)
{
    //painter->fillRect(rect(), backgroundColor);

    int canvasHeight = rect().height() - timescaleHeight;

    int process_spacing = 0;
    if (canvasHeight / processSpan > 12)
        process_spacing = 3;

    float x, y, w, h;
    float blockheight = floor(canvasHeight / processSpan);
    float barheight = blockheight - process_spacing;
    processheight = blockheight;
    startStep = maxStep;
    int stopStep = 0;

    painter->setFont(QFont("Helvetica", 10));
    QFontMetrics font_metrics = this->fontMetrics();

    int position, step;
    bool complete;
    QSet<Message *> drawMessages = QSet<Message *>();
    unsigned long long stopTime = startTime + timeSpan;
    painter->setPen(QPen(QColor(0, 0, 0)));
    drawnEvents.clear();
    for (QList<Partition*>::Iterator part = trace->partitions->begin(); part != trace->partitions->end(); ++part)
    {
        for (QMap<int, QList<Event *> *>::Iterator event_list = (*part)->events->begin(); event_list != (*part)->events->end(); ++event_list)
        {
            for (QList<Event *>::Iterator evt = (event_list.value())->begin(); evt != (event_list.value())->end(); ++evt)
            {
                position = proc_to_order[(*evt)->process];
                if ((*evt)->exit < startTime || (*evt)->enter > stopTime) // Out of time span
                    continue;
                if (position < floor(startProcess) || position > ceil(startProcess + processSpan)) // Out of span
                    continue;

                // save step information for emitting
                step = (*evt)->step;
                if (step >= 0 && step > stopStep)
                    stopStep = step;
                if (step >= 0 && step < startStep) {
                    startStep = step;
                }


                w = ((*evt)->exit - (*evt)->enter) / 1.0 / timeSpan * rect().width();
                if (w < 2)
                    continue;

                y = floor((position - startProcess) * blockheight) + 1;
                x = floor(static_cast<long long>((*evt)->enter - startTime) / 1.0 / timeSpan * rect().width()) + 1;
                h = barheight;


                // Corrections for partially drawn
                complete = true;
                if (y < 0) {
                    h = barheight - fabs(y);
                    y = 0;
                    complete = false;
                } else if (y + barheight > canvasHeight) {
                    h = canvasHeight - y;
                    complete = false;
                }
                if (x < 0) {
                    w -= fabs(x);
                    x = 0;
                    complete = false;
                } else if (x + w > rect().width()) {
                    w = rect().width() - x;
                    complete = false;
                }


                // Change pen color if selected
                if (*evt == selected_event)
                {
                    // Draw event
                    painter->fillRect(QRectF(x, y, w, h), QBrush(Qt::yellow));
                    painter->setPen(QPen(Qt::yellow));
                }
                else
                    // Draw event
                    painter->fillRect(QRectF(x, y, w, h), QBrush(QColor(200, 200, 255)));
                // Draw border
                if (process_spacing > 0)
                    if (complete)
                        painter->drawRect(QRectF(x,y,w,h));
                    else
                        incompleteBox(painter, x, y, w, h);
                // Revert pen color
                if (*evt == selected_event)
                    painter->setPen(QPen(QColor(0, 0, 0)));

                drawnEvents[*evt] = QRect(x, y, w, h);

                QString fxnName = ((*(trace->functions))[(*evt)->function])->name;
                QRect fxnRect = font_metrics.boundingRect(fxnName);
                //std::cout << fxnRect.width() << ", " << fxnRect.height() << std::endl;
                if (fxnRect.width() < w && fxnRect.height() < h)
                    painter->drawText(x + 2, y + fxnRect.height() - 2, fxnName);

                for (QVector<Message *>::Iterator msg = (*evt)->messages->begin(); msg != (*evt)->messages->end(); ++msg)
                    drawMessages.insert((*msg));
            }
        }
    }

        // Messages
        // We need to do all of the message drawing after the event drawing
        // for overlap purposes
        painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
        Event * send_event;
        Event * recv_event;
        QPointF p1, p2;
        for (QSet<Message *>::Iterator msg = drawMessages.begin(); msg != drawMessages.end(); ++msg) {
            send_event = (*msg)->sender;
            recv_event = (*msg)->receiver;
            position = proc_to_order[send_event->process];
            y = floor((position - startProcess + 0.5) * blockheight) + 1;
            x = floor(static_cast<long long>(send_event->enter - startTime) / 1.0 / timeSpan * rect().width()) + 1;
            p1 = QPointF(x, y);
            position = proc_to_order[recv_event->process];
            y = floor((position - startProcess + 0.5) * blockheight) + 1;
            x = floor(static_cast<long long>(recv_event->exit - startTime) / 1.0 / timeSpan * rect().width()) + 1;
            p2 = QPointF(x, y);
            painter->drawLine(p1, p2);
        }

    stepSpan = stopStep - startStep;
}