#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QPoint>

#include "boost/multi_array.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    using AmplitudesType = double;

private:
    QLabel *m_image_label;
    QPixmap m_image_pixmap;
    qreal m_zoom_factor;

    std::vector<QPoint> m_picked_path;

    void openImage(const QString &file_name);
    void updateImage();
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // MAINWINDOW_H
