#include "viswidget.h"

VisWidget::VisWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    container(parent),
    trace(NULL),
    visProcessed(false),
    backgroundColor(Qt::white),
    selectColor(QBrush(Qt::yellow)),
    changeSource(false),
    border(20),
    drawnEvents(QMap<Event *, QRect>()),
    selected_event(NULL),
    hover_event(NULL),
    closed(false)
{
    // GLWidget options
    setMinimumSize(200, 200);
    setAutoFillBackground(true);
    setWindowTitle("");
}

VisWidget::~VisWidget()
{
}

QSize VisWidget::sizeHint() const
{
    return QSize(400, 400);
}

void VisWidget::initializeGL()
{
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH);
}

void VisWidget::setSteps(float start, float stop)
{
    Q_UNUSED(start);
    Q_UNUSED(stop);
}

void VisWidget::selectEvent(Event * evt)
{
    Q_UNUSED(evt);
}


void VisWidget::setTrace(Trace * t)
{
    trace = t;
}

void VisWidget::prepaint()
{

}

void VisWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    prepaint();

    // Clear
    qglClearColor(backgroundColor);
    glClear(GL_COLOR_BUFFER_BIT);

    beginNativeGL();
    {
        drawNativeGL();
    }
    endNativeGL();

    QPainter painter(this);
    //painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    qtPaint(&painter);
    painter.end();
}

void VisWidget::drawNativeGL()
{
}

void VisWidget::beginNativeGL()
{
    makeCurrent();

    int width2x = width()*2;
    int height2x = height()*2;

    glViewport(0, 0, width2x, height2x);

    // Switch for 2D drawing
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
}

void VisWidget::endNativeGL()
{
    // Revert settings for painter
    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void VisWidget::processVis()
{

}

void VisWidget::qtPaint(QPainter *painter)
{
    Q_UNUSED(painter);
}

void VisWidget::incompleteBox(QPainter *painter, float x, float y, float w, float h)
{
    bool left = true;
    bool right = true;
    bool top = true;
    bool bottom = true;
    if (x <= 0)
        left = false;
    if (x + w >= rect().width())
        right = false;
    if (y <= 0)
        top = false;
    if (y + h >= rect().height())
        bottom = false;

    if (left)
        painter->drawLine(QPointF(x, y), QPointF(x, y + h));

    if (right)
        painter->drawLine(QPointF(x + w, y), QPointF(x + w, y + h));

    if (top)
        painter->drawLine(QPointF(x, y), QPointF(x + w, y));

    if (bottom)
        painter->drawLine(QPointF(x, y + h), QPointF(x + w, y + h));
}


// If we want an odd step, we actually need the step after it since that is
// where in the information is stored. This function computes that.
int VisWidget::boundStep(float step) {
    int bstep = ceil(step);
    if (bstep % 2)
        bstep++;
    return bstep;
}

void VisWidget::setClosed(bool _closed)
{
    if (closed && !_closed)
    {
        repaint();
    }
    closed = _closed;
}
