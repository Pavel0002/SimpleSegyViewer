#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QPoint>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QLabel *m_image_label;
    QPixmap m_image;
    qreal m_zoom_factor;

    std::vector<QPoint> m_pickedPath;

    void openImage(const QString &file_name);
    void updateImage();
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // MAINWINDOW_H
