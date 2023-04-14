#include "mainwindow.h"
#include <QFileDialog>
#include <QtMath>

#include <math.h>

#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), zoomFactor(1.0)
{
    imageLabel = new QLabel(this);
    setCentralWidget(imageLabel);
    imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    openImage(QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp)")));
}

MainWindow::~MainWindow()
{
}

void MainWindow::openImage(const QString &fileName)
{
    image = QPixmap(fileName);

    if (!image.isNull()) {
        updateImage();
        //setFixedSize(image.size());
        //setMinimumSize(image.size());
    }
}

void MainWindow::updateImage()
{
    QPixmap scaledImage = image.scaled(image.size() * zoomFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    imageLabel->setPixmap(scaledImage);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8;

    qreal newZoomFactor = 1.0;
    qreal scalingFactor = 0.125;
    if(numDegrees > 0)
    {
        newZoomFactor = zoomFactor + zoomFactor * scalingFactor;
    }
    else
    {
        newZoomFactor = zoomFactor - zoomFactor * scalingFactor;
    }

    if (newZoomFactor != zoomFactor) {
        zoomFactor = newZoomFactor;
        updateImage();
    }

    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !image.isNull()) {
        QPoint pos = imageLabel->mapFrom(this, event->pos()); // map mouse click to imageLabel coordinates
        pos /= zoomFactor; // adjust for zoom level
        if (image.rect().contains(pos)) { // check if the clicked position is within the image bounds
            // Get the color of the clicked pixel
            QRgb color = image.toImage().pixel(pos);
            int red = qRed(color);
            int green = qGreen(color);
            int blue = qBlue(color);
            qDebug() << "Clicked on pixel at (" << pos.x() << "," << pos.y() << "), color: (" << red << "," << green << "," << blue << ")";

            // Modify the color of the clicked pixel
            QImage modifiedImage = image.toImage();
            modifiedImage.setPixel(pos, qRgb(0, 0, 0));
            image = QPixmap::fromImage(modifiedImage);
            updateImage();
        }
    }

    QWidget::mousePressEvent(event);
}
