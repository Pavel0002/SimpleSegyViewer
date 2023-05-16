#include "mainwindow.h"

#include <QFileDialog>
#include <QtMath>
#include <QDebug>

#include <Windows.h>
#undef max // conflicts with std::numeric_limits<PixelType>::max()

#include <fstream>
#include <string>

#include "boost/multi_array.hpp"

// Windows OS only
std::string MainWindow::getCurrentDirectory()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
	const std::string::size_type position = std::string(buffer).find_last_of("\\/");

    return std::string(buffer).substr(0, position);
}

void MainWindow::readGatherFromBinaryFile(const std::string &filename)
{
    std::ifstream input_file(filename, std::ios::binary);

    // Read the number of rows and columns from the file
    int samples_count;
	int traces_count;
    input_file.read(reinterpret_cast<char *>(&samples_count), sizeof(samples_count));
    input_file.read(reinterpret_cast<char *>(&traces_count), sizeof(traces_count));

    m_gather_2d.resize(boost::extents[samples_count][traces_count]);

    // Read the data from the file into the array
    for (int i = 0; i < samples_count; i++)
    {
        for (int j = 0; j < traces_count; j++)
        {
            float value;
            input_file.read(reinterpret_cast<char *>(&value), sizeof(value));
            m_gather_2d[i][j] = static_cast<double>(value);
        }
    }
}

unsigned char MainWindow::convertSampleToPixel(SampleType sample) const
{
	const SampleType samples_range_length = (m_max_sample_in_original_gather - m_min_sample_in_original_gather);
    PixelType pixel_value = static_cast<PixelType>(0);
	if (samples_range_length > std::numeric_limits<SampleType>::epsilon())
    {
        SampleType converted_sample =
            ((sample - m_min_sample_in_original_gather) / samples_range_length) *
            static_cast<SampleType>(std::numeric_limits<PixelType>::max());
        pixel_value = static_cast<PixelType>(std::round(converted_sample));
	}

    return pixel_value;
}

void MainWindow::deletePickedEvent()
{
    //must be odd?
    const int event_time_width_in_samples = 11;

    // TODO: implement this
}

//// Writes a boost array of floats to a binary file
//void write_binary_file(const std::string &filename, const boost::multi_array<float, 2> &array)
//{
//    std::ofstream output_file(filename, std::ios::binary);
//
//    // Write the number of rows and columns to the file
//    int numRows = array.shape()[0];
//    int numCols = array.shape()[1];
//    output_file.write(reinterpret_cast<const char *>(&numRows), sizeof(numRows));
//    output_file.write(reinterpret_cast<const char *>(&numCols), sizeof(numCols));
//
//    // Write the array data to the file
//    for (int i = 0; i < numRows; i++)
//    {
//        for (int j = 0; j < numCols; j++)
//        {
//            float value = array[i][j];
//            output_file.write(reinterpret_cast<const char *>(&value), sizeof(value));
//        }
//    }
//}

void MainWindow::convertSegyToBinaryFile(const QString &native_separators_filename, const QString &binary_filename)
{
	const QString this_app_executable_directory =
		QDir::toNativeSeparators(QString::fromStdString(getCurrentDirectory()));
	const QString segy_utility_name = "SegySamplesExtractorInserter.exe";
	QString segy_utility_full_path = 
		QDir::toNativeSeparators(this_app_executable_directory + QDir::separator() + segy_utility_name);

	// TODO: there are better ways to launch exe files (maybe CreateProcess?)
	const QString from_segy_command = "FromSegyToBinary";
	const QString command_to_exe =
		segy_utility_full_path + " " + native_separators_filename + " " + binary_filename + " " + from_segy_command;
	// Note: currently are .Net Framework exe files are used
	int segy_utility_result = std::system(qUtf8Printable(command_to_exe));
}

void MainWindow::openSegy(const QString &filename)
{
    const QString native_separators_filename = QDir::toNativeSeparators(filename);
	const QFileInfo info(native_separators_filename);
	const QString binary_filename = 
        QDir::toNativeSeparators(info.path() + QDir::separator() + info.completeBaseName() + ".bin");

	convertSegyToBinaryFile(native_separators_filename, binary_filename);

	readGatherFromBinaryFile(binary_filename.toStdString());

    // it's important to save original minimum and maximum,
    // be because the gather may change, and this values are used to
    // render the gather. If this values doesn't change, the rendered
    // results are comparable.
    m_max_sample_in_original_gather = 
        *std::max_element(m_gather_2d.origin(), m_gather_2d.origin() + m_gather_2d.num_elements());
    m_min_sample_in_original_gather = 
        *std::min_element(m_gather_2d.origin(), m_gather_2d.origin() + m_gather_2d.num_elements());

	updateImageDataFromGather();
    if (!m_image_pixmap.isNull())
    {
        updateImageOnScreen();
    }
}

MainWindow::MainWindow(QWidget *parent):
QMainWindow(parent),
m_zoom_factor_width(1.0),
m_zoom_factor_height(1.0)
{
    m_p_image_label = new QLabel(this);
    setCentralWidget(m_p_image_label);
    m_p_image_label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    //m_p_image_label->setAlignment(Qt::AlignJustify);

    // TODO: подумать над вариантом работы из картинки //const QString from_segy_command = "FromBinaryToSegy";
    // when image is opened max and min of seismic data can be derived from image somehow (average is zero???)
    openImage(QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp)")));

	openSegy(QFileDialog::getOpenFileName(this, tr("Open Seg-y"), ".", tr("Seg-y Files (*.sgy *.segy)")));

}

MainWindow::~MainWindow()
{
}

void MainWindow::openImage(const QString &filename)
{
    initializePixmap(QPixmap(filename));

    // todo: should we check it for null?
    if (!m_image_pixmap.isNull()) 
    {
        updateImageOnScreen();
        //setFixedSize(m_image_pixmap.size());
        //setMinimumSize(m_image_pixmap.size());

        updateGatherDataFromImage();
    }
}

void MainWindow::updateImageOnScreen()
{
    const int scaled_width = static_cast<int>(std::round(m_image_pixmap.width() * m_zoom_factor_width));
    const int scaled_height = static_cast<int>(std::round(m_image_pixmap.height() * m_zoom_factor_height));
    m_image_pixmap_transformed =
        m_image_pixmap.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	m_p_image_label->setPixmap(m_image_pixmap_transformed);
}

void MainWindow::initializePixmap(const QPixmap &image_pixmap)
{
	m_image_pixmap = image_pixmap;
	m_image_pixmap_transformed = m_image_pixmap;
}

void MainWindow::updateImageDataFromGather()
{
    // TODO:
    // если пользователь не выберет файл обработать ситуацию!
    // надо проработать логику с учетом возоможности скалирования
    // сделать, чтобы скалирование оставалось прежним после работы этой функции
    // переделать под поддержку только обесцвеченных изображений?
    // сделать, что если подается цветное, то оно обесцвечивается!
    // наверное эти параметры не должны меняться в течении одной сессии, может сделать их членами класса
    // and rename to traces and samples
    const int traces_count = static_cast<int>(m_gather_2d.shape()[0]);
    const int samples_per_trace_count = static_cast<int>(m_gather_2d.shape()[1]);

	QImage image(traces_count, samples_per_trace_count, QImage::Format_Grayscale8);
    for (int trace_index = 0; trace_index < traces_count; ++trace_index)
    {
        for (int sample_index = 0; sample_index < samples_per_trace_count; ++sample_index)
        {
			const PixelType pixel_value = convertSampleToPixel(m_gather_2d[trace_index][sample_index]);
            image.setPixel(trace_index, sample_index, qRgb(pixel_value, pixel_value, pixel_value));
        }
    }

	QPixmap image_pixmap = QPixmap::fromImage(image);
	initializePixmap(image_pixmap);
}

void MainWindow::updateGatherDataFromImage()
{
    QImage image = m_image_pixmap.toImage();

    const int traces_count = image.width();
	const int samples_per_trace_count = image.height();

    m_gather_2d.resize(boost::extents[traces_count][samples_per_trace_count]);

    for (int trace_index = 0; trace_index < image.width(); ++trace_index)
    {
        for (int sample_index = 0; sample_index < image.height(); ++sample_index)
        {
            QRgb color = image.pixel(trace_index, sample_index);
			const int gray = qGray(color);
            m_gather_2d[trace_index][sample_index] = static_cast<unsigned char>(gray);
        }
    }
}

void MainWindow::scaleImage(const ScaleType scale_type, const Direction direction)
{
    const qreal multiplier = (direction == Direction::Increase) ? m_scaling_term : (1 / m_scaling_term);
	switch (scale_type) {
	case ScaleType::Height:
        m_zoom_factor_height *= multiplier;
		break;
	case ScaleType::Width:
        m_zoom_factor_width *= multiplier;
		break;
	case ScaleType::All:
        m_zoom_factor_width *= multiplier;
        m_zoom_factor_height *= multiplier;
		break;
	default: 
        Q_UNREACHABLE();
	}

    updateImageOnScreen();
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
	const int num_degrees = event->angleDelta().y() / 8;

    //TODO: refactor this
    if(num_degrees > 0)
    {
        scaleImage(ScaleType::All, Direction::Increase);
    }
    else
    {
        scaleImage(ScaleType::All, Direction::Decrease);
    }

    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
	{
    case Qt::Key_Left:
        scaleImage(ScaleType::Width, Direction::Decrease);
        break;
    case Qt::Key_Right:
        scaleImage(ScaleType::Width, Direction::Increase);
        break;
    case Qt::Key_Down:
        scaleImage(ScaleType::Height, Direction::Decrease);
        break;
    case Qt::Key_Up:
        scaleImage(ScaleType::Height, Direction::Increase);
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::MiddleButton && !m_image_pixmap.isNull()) {
		QPoint diff = event->pos() - m_last_drag_position;
		m_last_drag_position = event->pos();

		// Move the image label by the drag amount
		m_label_position = m_p_image_label->pos() + diff;
		m_p_image_label->move(m_label_position);
	}

	QWidget::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{

    // TODO: broken after independent axes scaling was added
    if (event->button() == Qt::LeftButton && !m_image_pixmap.isNull()) 
    {
        QPoint position = m_p_image_label->mapFrom(this, event->pos()); // map mouse click to m_p_image_label coordinates

        position.rx() = std::round(position.rx() / m_zoom_factor_width);
        position.ry() = std::round(position.ry() / m_zoom_factor_height);

        if (m_image_pixmap.rect().contains(position)) { // check if the clicked position is within the m_image_pixmap bounds
            // Get the color of the clicked pixel
            QRgb color = m_image_pixmap.toImage().pixel(position);
            int red = qRed(color);
            int green = qGreen(color);
            int blue = qBlue(color);
            qDebug() << "Clicked on pixel at (" << position.x() << "," << position.y() << "), color: (" << red << "," << green << "," << blue << ")";

            m_picked_path.emplace_back(QPoint(position.x(), position.y()));

            // For debug: Modify the color of the clicked pixel
            QImage modified_image = m_image_pixmap.toImage();
            modified_image.setPixel(position, qRgb(0, 0, 0));
            m_image_pixmap = QPixmap::fromImage(modified_image);

            updateImageOnScreen();
        }
    }
    else if (event->button() == Qt::MiddleButton && !m_image_pixmap.isNull()) {
        m_is_dragging = true;
        m_last_drag_position = event->pos();
    }

    QWidget::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::MiddleButton && !m_image_pixmap.isNull()) {
		m_is_dragging = false;
	}

	QWidget::mouseReleaseEvent(event);
}
