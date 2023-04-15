#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QPoint>

#include "boost/multi_array.hpp"

// TODO:
//only grayscale images should be supported!
// add logic of conversion from seismic to image and back with correct transform
// of amplitudes
//2. сглаживание вдоль пути ширина определенная.
//3. окно ширины, -параметр
//4. загрузка файла и его сохранение в формате картинки и seg-y
//независимый зум по осям


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // todo: maybe private?
    void wheelEvent(QWheelEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    using SampleType = double;
    using PixelType = unsigned char;
    using Gather2d = boost::multi_array<SampleType, 2>;

	enum class ScaleType
	{
		Height,
		Width,
        All
	};

    enum class Direction
    {
        Decrease,
        Increase
	};

private:
    QLabel *m_p_image_label;// todo: should it be a pointer?

    QPixmap m_image_pixmap;
    QPixmap m_image_pixmap_transformed;
    Gather2d m_gather_2d;

    // todo: maybe const?
    SampleType m_max_sample_in_original_gather = static_cast<SampleType>(0.0);
    SampleType m_min_sample_in_original_gather = static_cast<SampleType>(0.0);

    std::vector<QPoint> m_picked_path;

    const qreal m_scaling_term = 1.125;// TODO: use not term, but multiplier
    qreal m_zoom_factor_width;
    qreal m_zoom_factor_height;

    QPoint m_last_drag_position;
    QPoint m_label_position;
    bool m_is_dragging = false;

    void openImage(const QString &filename);
	void openSegy(const QString &filename);

    static void convertSegyToBinaryFile(const QString &native_separators_filename, const QString &binary_filename);
    void readGatherFromBinaryFile(const std::string &filename);

    void initializePixmap(const QPixmap &image_pixmap);

	void updateImageOnScreen();
    //void updateImageOnScreenCT();
	void updateImageDataFromGather();
	void updateGatherDataFromImage();

	void scaleImage(ScaleType scale_type, Direction direction);

	void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

	static std::string getCurrentDirectory();

    PixelType convertSampleToPixel(SampleType sample) const;

	void deletePickedEvent();
};

#endif // MAINWINDOW_H
