#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QLabel *imageLabel;
    QPixmap image;
    qreal zoomFactor;

    void openImage(const QString &fileName);
    void updateImage();
    void mousePressEvent(QMouseEvent *event) override;

};

#endif // MAINWINDOW_H
