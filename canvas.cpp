#include "canvas.h"

#include <QMouseEvent>
#include <QPainter>

#include <algorithm>
#include <cmath>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent), image_(900, 620, QImage::Format_ARGB32) {
    image_.fill(Qt::white);
    setMinimumSize(image_.size());
    setCursor(Qt::CrossCursor);
}

void Canvas::setMode(Mode mode) {
    mode_ = mode;
}

void Canvas::setBoundaryColor(const QColor &color) {
    boundaryColor_ = color;
}

void Canvas::setFillColor(const QColor &color) {
    fillColor_ = color;
}

void Canvas::clear() {
    image_.fill(Qt::white);
    update();
}

void Canvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(238, 241, 245));
    painter.drawImage(0, 0, image_);
    painter.setPen(QPen(QColor(90, 100, 112), 1));
    painter.drawRect(image_.rect().adjusted(0, 0, -1, -1));
}

void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) {
        return;
    }

    const QPoint point = event->position().toPoint();
    if (!inside(point)) {
        return;
    }

    if (mode_ == Mode::Fill) {
        fillFrom(point);
        update();
        return;
    }

    mouseDown_ = true;
    previousPoint_ = point;
    lineStart_ = point;
    if (mode_ == Mode::DrawBoundary) {
        putPixel(point.x(), point.y(), boundaryColor_);
        update();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (!mouseDown_ || mode_ != Mode::DrawBoundary) {
        return;
    }

    const QPoint point = bounded(event->position().toPoint());
    drawBresenham(previousPoint_, point, boundaryColor_);
    previousPoint_ = point;
    update();
}

void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton || !mouseDown_) {
        return;
    }

    const QPoint point = bounded(event->position().toPoint());
    if (mode_ == Mode::DrawBoundary) {
        drawBresenham(previousPoint_, point, boundaryColor_);
    } else if (mode_ == Mode::BresenhamLine) {
        drawBresenham(lineStart_, point, boundaryColor_);
    }
    mouseDown_ = false;
    update();
}

bool Canvas::inside(const QPoint &point) const {
    return image_.rect().contains(point);
}

QPoint Canvas::bounded(const QPoint &point) const {
    return {std::clamp(point.x(), 0, image_.width() - 1),
            std::clamp(point.y(), 0, image_.height() - 1)};
}

void Canvas::putPixel(int x, int y, const QColor &color) {
    if (x >= 0 && x < image_.width() && y >= 0 && y < image_.height()) {
        image_.setPixelColor(x, y, color);
    }
}

// Егор: целочисленный Брезенхем для любых направлений отрезка.
void Canvas::drawBresenham(QPoint from, const QPoint &to, const QColor &color) {
    int x0 = from.x();
    int y0 = from.y();
    const int x1 = to.x();
    const int y1 = to.y();
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        putPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        const int doubledError = 2 * error;
        if (doubledError >= dy) {
            error += dy;
            x0 += sx;
        }
        if (doubledError <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

void Canvas::fillFrom(const QPoint &seed) {
    const QRgb oldColor = image_.pixel(seed);
    const QRgb newColor = fillColor_.rgba();
    if (oldColor == newColor) {
        return;
    }
    fillSpan(seed.x(), seed.y(), oldColor, newColor);
}

// Егор: рекурсивная scanline-заливка. Один вызов сразу красит всю серию.
void Canvas::fillSpan(int x, int y, QRgb oldColor, QRgb newColor) {
    if (x < 0 || x >= image_.width() || y < 0 || y >= image_.height()
        || image_.pixel(x, y) != oldColor) {
        return;
    }

    int left = x;
    int right = x;
    while (left > 0 && image_.pixel(left - 1, y) == oldColor) {
        --left;
    }
    while (right + 1 < image_.width() && image_.pixel(right + 1, y) == oldColor) {
        ++right;
    }
    for (int currentX = left; currentX <= right; ++currentX) {
        image_.setPixel(currentX, y, newColor);
    }

    // На соседних строках запускаемся один раз для каждой новой серии.
    for (const int nextY : {y - 1, y + 1}) {
        if (nextY < 0 || nextY >= image_.height()) {
            continue;
        }
        int currentX = left;
        while (currentX <= right) {
            while (currentX <= right && image_.pixel(currentX, nextY) != oldColor) {
                ++currentX;
            }
            if (currentX <= right) {
                fillSpan(currentX, nextY, oldColor, newColor);
                while (currentX <= right && image_.pixel(currentX, nextY) != oldColor) {
                    ++currentX;
                }
            }
        }
    }
}
