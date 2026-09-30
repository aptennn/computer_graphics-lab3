#include "mainwindow.h"

#include "canvas.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Компьютерная графика — заливка и отрезки"));

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    auto *toolbar = new QVBoxLayout;
    auto *modeRow = new QHBoxLayout;
    auto *actionRow = new QHBoxLayout;
    auto *canvas = new Canvas(central);
    auto *canvasScrollArea = new QScrollArea(central);
    canvasScrollArea->setWidget(canvas);
    canvasScrollArea->setWidgetResizable(false);
    canvasScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    auto *drawMode = new QRadioButton(QStringLiteral("Рисовать границу"), central);
    auto *fillMode = new QRadioButton(QStringLiteral("Заливка сериями"), central);
    auto *textureMode = new QRadioButton(QStringLiteral("Заливка изображением"), central);
    auto *lineMode = new QRadioButton(QStringLiteral("Отрезок Брезенхема"), central);
    auto *wuMode = new QRadioButton(QStringLiteral("Отрезок Ву"), central);
    drawMode->setChecked(true);
    textureMode->setEnabled(false);

    auto *modes = new QButtonGroup(this);
    modes->addButton(drawMode);
    modes->addButton(fillMode);
    modes->addButton(textureMode);
    modes->addButton(lineMode);
    modes->addButton(wuMode);

    auto *boundaryColor = new QPushButton(QStringLiteral("Цвет границы"), central);
    auto *fillColor = new QPushButton(QStringLiteral("Цвет заливки"), central);
    auto *loadTexture = new QPushButton(QStringLiteral("Загрузить изображение"), central);
    auto *textureName = new QLabel(QStringLiteral("Файл не выбран"), central);
    auto *clearButton = new QPushButton(QStringLiteral("Очистить"), central);

    modeRow->addWidget(drawMode);
    modeRow->addWidget(fillMode);
    modeRow->addWidget(textureMode);
    modeRow->addWidget(lineMode);
    modeRow->addWidget(wuMode);
    modeRow->addStretch();
    actionRow->addWidget(boundaryColor);
    actionRow->addWidget(fillColor);
    actionRow->addWidget(loadTexture);
    actionRow->addWidget(textureName);
    actionRow->addStretch();
    actionRow->addWidget(clearButton);
    toolbar->addLayout(modeRow);
    toolbar->addLayout(actionRow);

    auto *hint = new QLabel(
        QStringLiteral("Нарисуйте замкнутую область (и внутренние контуры-отверстия), "
                       "затем выберите заливку и щёлкните внутри. Для заливки изображением "
                       "сначала загрузите файл. Для отрезка — протяните мышь."),
        central);
    hint->setWordWrap(true);
    auto *author = new QLabel(
        QStringLiteral("Егор — задание 1а, Брезенхем и основа интерфейса; "
                       "Михаил — задание 1б и алгоритм Ву"),
        central);
    author->setStyleSheet(QStringLiteral("color: #53606f; font-size: 12px;"));

    rootLayout->addLayout(toolbar);
    rootLayout->addWidget(hint);
    rootLayout->addWidget(canvasScrollArea, 1);
    rootLayout->addWidget(author);
    setCentralWidget(central);

    connect(drawMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::DrawBoundary);
    });
    connect(fillMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::Fill);
    });
    connect(textureMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::TextureFill);
    });
    connect(lineMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::BresenhamLine);
    });
    connect(wuMode, &QRadioButton::toggled, this, [canvas](bool checked) {
        if (checked) canvas->setMode(Canvas::Mode::WuLine);
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
    // Михаил: загрузка рисунка из файла для заливки без изменения его размера.
    connect(loadTexture, &QPushButton::clicked, this, [this, canvas, textureMode, textureName] {
        const QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Выберите изображение для заливки"), QString(),
            QStringLiteral("Изображения (*.png *.jpg *.jpeg *.bmp *.gif *.webp *.tif *.tiff);;Все файлы (*)"));
        if (path.isEmpty()) {
            return;
        }
        QImage texture(path);
        if (texture.isNull()) {
            QMessageBox::warning(this, QStringLiteral("Ошибка загрузки"),
                                 QStringLiteral("Не удалось прочитать изображение: %1").arg(path));
            return;
        }
        canvas->setTexture(texture);
        textureName->setText(QFileInfo(path).fileName());
        textureName->setToolTip(path);
        textureMode->setEnabled(true);
        textureMode->setChecked(true);
        statusBar()->showMessage(QStringLiteral("Загружено изображение %1 (%2 × %3)")
                                     .arg(QFileInfo(path).fileName())
                                     .arg(texture.width())
                                     .arg(texture.height()));
    });
    connect(clearButton, &QPushButton::clicked, canvas, &Canvas::clear);

    statusBar()->showMessage(QStringLiteral("Режим: рисование границы"));
    connect(drawMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: рисование границы"));
    });
    connect(fillMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: заливка сериями пикселей"));
    });
    connect(textureMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: заливка изображением из файла"));
    });
    connect(lineMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: целочисленный Брезенхем"));
    });
    connect(wuMode, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) statusBar()->showMessage(QStringLiteral("Режим: алгоритм Ву"));
    });

    resize(1100, 800);
}
