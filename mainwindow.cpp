#include "mainwindow.h"

#include "canvas.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Компьютерная графика — заливка и Брезенхем"));

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    auto *toolbar = new QHBoxLayout;
    auto *canvas = new Canvas(central);

    auto *drawMode = new QRadioButton(QStringLiteral("Рисовать границу"), central);
    auto *fillMode = new QRadioButton(QStringLiteral("Заливка сериями"), central);
    auto *lineMode = new QRadioButton(QStringLiteral("Отрезок Брезенхема"), central);
    drawMode->setChecked(true);

    auto *modes = new QButtonGroup(this);
    modes->addButton(drawMode);
    modes->addButton(fillMode);
    modes->addButton(lineMode);

    auto *boundaryColor = new QPushButton(QStringLiteral("Цвет границы"), central);
    auto *fillColor = new QPushButton(QStringLiteral("Цвет заливки"), central);
    auto *clearButton = new QPushButton(QStringLiteral("Очистить"), central);

    toolbar->addWidget(drawMode);
    toolbar->addWidget(fillMode);
    toolbar->addWidget(lineMode);
    toolbar->addSpacing(16);
    toolbar->addWidget(boundaryColor);
    toolbar->addWidget(fillColor);
    toolbar->addStretch();
    toolbar->addWidget(clearButton);

    auto *hint = new QLabel(
        QStringLiteral("Нарисуйте замкнутую область (и внутренние контуры-отверстия), "
                       "затем выберите заливку и щёлкните внутри. Для отрезка — протяните мышь."),
        central);
    hint->setWordWrap(true);
    auto *author = new QLabel(
        QStringLiteral("Егор — задание 1а, целочисленный алгоритм Брезенхема и основа интерфейса"),
        central);
    author->setStyleSheet(QStringLiteral("color: #53606f; font-size: 12px;"));

    rootLayout->addLayout(toolbar);
    rootLayout->addWidget(hint);
    rootLayout->addWidget(canvas, 1, Qt::AlignHCenter);
    rootLayout->addWidget(author);
    setCentralWidget(central);

    connect(drawMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::DrawBoundary);
    });
    connect(fillMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::Fill);
    });
    connect(lineMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::BresenhamLine);
    });
    connect(boundaryColor, &QPushButton::clicked, this, [this, canvas] {
        const QColor color = QColorDialog::getColor(Qt::black, this, QStringLiteral("Цвет границы"));
        if (color.isValid()) canvas->setBoundaryColor(color);
    });
    connect(fillColor, &QPushButton::clicked, this, [this, canvas] {
        const QColor color = QColorDialog::getColor(QColor(65, 145, 255), this,
                                                    QStringLiteral("Цвет заливки"));
        if (color.isValid()) canvas->setFillColor(color);
    });
    connect(clearButton, &QPushButton::clicked, canvas, &Canvas::clear);

    statusBar()->showMessage(QStringLiteral("Режим: рисование границы"));
    connect(drawMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: рисование границы"));
    });
    connect(fillMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: заливка сериями пикселей"));
    });
    connect(lineMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: целочисленный Брезенхем"));
    });

    resize(1040, 760);
}
