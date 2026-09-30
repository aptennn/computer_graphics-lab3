#pragma once

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;

class Canvas final : public QWidget {
public:
    enum class Mode { DrawBoundary, Fill, BresenhamLine };

    explicit Canvas(QWidget *parent = nullptr);

    void setMode(Mode mode);
    void setBoundaryColor(const QColor &color);
    void setFillColor(const QColor &color);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    [[nodiscard]] bool inside(const QPoint &point) const;
    [[nodiscard]] QPoint bounded(const QPoint &point) const;

    void putPixel(int x, int y, const QColor &color);
    void drawBresenham(QPoint from, const QPoint &to, const QColor &color);
    void fillFrom(const QPoint &seed);
    void fillSpan(int x, int y, QRgb oldColor, QRgb newColor);

    QImage image_;
    Mode mode_ = Mode::DrawBoundary;
    QColor boundaryColor_ = Qt::black;
    QColor fillColor_ = QColor(65, 145, 255);
    QPoint previousPoint_;
    QPoint lineStart_;
    bool mouseDown_ = false;
};
