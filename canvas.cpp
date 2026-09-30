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

void Canvas::setTexture(const QImage &texture) {
    texture_ = texture.convertToFormat(QImage::Format_ARGB32);
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
    if (mode_ == Mode::TextureFill) {
        if (!texture_.isNull()) {
            fillTextureFrom(point);
            update();
        }
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
    } else if (mode_ == Mode::WuLine) {
        drawWu(lineStart_, point, boundaryColor_);
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

// Михаил: смешивание с фоном по доле покрытия пикселя для алгоритма Ву и прозрачных текстур.
void Canvas::blendPixel(int x, int y, const QColor &color, double coverage) {
    if (x < 0 || x >= image_.width() || y < 0 || y >= image_.height()) {
        return;
    }
    const double alpha = std::clamp(coverage * color.alphaF(), 0.0, 1.0);
    if (alpha == 0.0) {
        return;
    }
    const QColor background = image_.pixelColor(x, y);
    image_.setPixel(x, y, qRgb(
        qRound(color.red() * alpha + background.red() * (1.0 - alpha)),
        qRound(color.green() * alpha + background.green() * (1.0 - alpha)),
        qRound(color.blue() * alpha + background.blue() * (1.0 - alpha))));
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

// Михаил: алгоритм Ву. Для каждого шага по главной оси распределяем яркость
// между двумя ближайшими пикселями пропорционально покрытию отрезком.
void Canvas::drawWu(QPoint from, QPoint to, const QColor &color) {
    bool steep = std::abs(to.y() - from.y()) > std::abs(to.x() - from.x());
    if (steep) {
        from = QPoint(from.y(), from.x());
        to = QPoint(to.y(), to.x());
    }
    if (from.x() > to.x()) {
        std::swap(from, to);
    }

    const auto plot = [this, steep, &color](int x, int y, double coverage) {
        if (steep) {
            blendPixel(y, x, color, coverage);
        } else {
            blendPixel(x, y, color, coverage);
        }
    };

    const int dx = to.x() - from.x();
    if (dx == 0) {
        plot(from.x(), from.y(), 1.0);
        return;
    }
    const double gradient = static_cast<double>(to.y() - from.y()) / dx;
    plot(from.x(), from.y(), 1.0);
    double y = from.y() + gradient;
    for (int x = from.x() + 1; x < to.x(); ++x, y += gradient) {
        const int lower = static_cast<int>(std::floor(y));
        const double fraction = y - lower;
        plot(x, lower, 1.0 - fraction);
        plot(x, lower + 1, fraction);
    }
    plot(to.x(), to.y(), 1.0);
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

void Canvas::fillTextureFrom(const QPoint &seed) {
    const QRgb oldColor = image_.pixel(seed);
    QImage visited(image_.size(), QImage::Format_Grayscale8);
    visited.fill(0);
    fillTextureSpan(seed.x(), seed.y(), oldColor, visited);
}

// Михаил: рекурсивная заливка текстурой по сериям. Маска посещения нужна, потому что пиксель
// текстуры может совпасть с исходным цветом или быть прозрачным.
void Canvas::fillTextureSpan(int x, int y, QRgb oldColor, QImage &visited) {
    const auto available = [this, oldColor, &visited](int px, int py) {
        return px >= 0 && px < image_.width() && py >= 0 && py < image_.height()
               && visited.constScanLine(py)[px] == 0 && image_.pixel(px, py) == oldColor;
    };
    if (!available(x, y)) {
        return;
    }

    int left = x;
    int right = x;
    while (available(left - 1, y)) {
        --left;
    }
    while (available(right + 1, y)) {
        ++right;
    }
    for (int px = left; px <= right; ++px) {
        visited.scanLine(y)[px] = 1;
        // Привязка к холсту: маленькая текстура повторяется по обеим осям,
        // большая используется в исходном размере без масштабирования.
        const QRgb texel = texture_.pixel(px % texture_.width(), y % texture_.height());
        blendPixel(px, y, QColor::fromRgba(texel), 1.0);
    }

    for (const int nextY : {y - 1, y + 1}) {
        if (nextY < 0 || nextY >= image_.height()) {
            continue;
        }
        int px = left;
        while (px <= right) {
            while (px <= right && !available(px, nextY)) {
                ++px;
            }
            if (px <= right) {
                fillTextureSpan(px, nextY, oldColor, visited);
                while (px <= right && !available(px, nextY)) {
                    ++px;
                }
            }
        }
    }
}
