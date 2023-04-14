#include "mainwindow.h"
#include <QFileDialog>
#include <QtMath>

#include <math.h>

#include "QDebug"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_zoom_factor(1.0)
{
    m_image_label = new QLabel(this);
    setCentralWidget(m_image_label);
    m_image_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    openImage(QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp)")));
}

MainWindow::~MainWindow()
{
}

void MainWindow::openImage(const QString &file_name)
{
    m_image = QPixmap(file_name);

    if (!m_image.isNull()) {
        updateImage();
        //setFixedSize(m_image.size());
        //setMinimumSize(m_image.size());
    }
}

void MainWindow::updateImage()
{
    QPixmap scaledImage = m_image.scaled(m_image.size() * m_zoom_factor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_image_label->setPixmap(scaledImage);
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    int num_degrees = event->angleDelta().y() / 8;

    qreal new_zoom_factor = 1.0;
    qreal scaling_factor = 0.125;
    if(num_degrees > 0)
    {
        new_zoom_factor = m_zoom_factor + m_zoom_factor * scaling_factor;
    }
    else
    {
        new_zoom_factor = m_zoom_factor - m_zoom_factor * scaling_factor;
    }

    if (new_zoom_factor != m_zoom_factor) {
        m_zoom_factor = new_zoom_factor;
        updateImage();
    }

    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !m_image.isNull()) {
        QPoint position = m_image_label->mapFrom(this, event->pos()); // map mouse click to m_image_label coordinates
        position /= m_zoom_factor; // adjust for zoom level
        if (m_image.rect().contains(position)) { // check if the clicked position is within the m_image bounds
            // Get the color of the clicked pixel
            QRgb color = m_image.toImage().pixel(position);
            int red = qRed(color);
            int green = qGreen(color);
            int blue = qBlue(color);
            qDebug() << "Clicked on pixel at (" << position.x() << "," << position.y() << "), color: (" << red << "," << green << "," << blue << ")";

            // Modify the color of the clicked pixel
            QImage modified_image = m_image.toImage();
            modified_image.setPixel(position, qRgb(0, 0, 0));
            m_image = QPixmap::fromImage(modified_image);
            updateImage();
        }
    }

    QWidget::mousePressEvent(event);
}
