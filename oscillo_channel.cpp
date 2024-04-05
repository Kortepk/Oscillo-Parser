#include "oscillo_channel.h"
#include <cmath>
#include <QValueAxis>

oscillo_channel::oscillo_channel()
    : Series_pointer(),
    ChartView_pointer()
{


}

void oscillo_channel::initSine()
{
    Series_pointer.clear();

    float i;

    for(i=0; i <= M_PI * 6; i+=0.05)
        ListPoint << QPointF(i, 2.5 * sin(i));

    ListPoint<< QPointF(i, 0); // Последнее значение

    Series_pointer.replace(ListPoint);

    ChartView_pointer.chart()->legend()->hide();
    ChartView_pointer.chart()->addSeries(&Series_pointer);
    ChartView_pointer.chart()->createDefaultAxes();
}

void oscillo_channel::ReplaceDots()
{
    Series_pointer.replace(ListPoint);
}

void oscillo_channel::SetGraphAxis(float *min_x, float *min_y, float *max_x, float *max_y)
{
    QtCharts::QChart *chart = ChartView_pointer.chart();

    // Удаление предыдущей оси X и отдельно Y
    QtCharts::QAbstractAxis *oldAxisX = chart->axes(Qt::Horizontal).at(0); // Получаем первую ось X
    chart->removeAxis(oldAxisX); // Удаляем ось X из графика
    delete oldAxisX; // Освобождаем память, выделенную для предыдущей оси X

    QtCharts::QAbstractAxis *oldAxisY = chart->axes(Qt::Vertical).at(0); // Получаем первую ось Y
    chart->removeAxis(oldAxisY); // Удаляем ось Y из графика
    delete oldAxisY; // Освобождаем память, выделенную для предыдущей оси Y

    // Создаем ось X и Y с заданными границами
    QtCharts::QValueAxis *axisX = new QtCharts::QValueAxis();
    axisX->setRange(*min_x, *max_x);
    chart->addAxis(axisX, Qt::AlignBottom);

    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setRange(*min_y, *max_y);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Устанавливаем ось X и ось Y для каждого QLineSeries (привязываем точки к изменённой оси)
    Series_pointer.attachAxis(axisX);
    Series_pointer.attachAxis(axisY);
}

void oscillo_channel::ClearPoints()
{
     Series_pointer.clear();
     ListPoint.clear();
}

void oscillo_channel::ChangeGroupSize(int val)
{
    QGroupBox_pointer->setMinimumSize(0, val);
    QGroupBox_pointer->setFixedHeight(val);
}

void oscillo_channel::CreateGroupBox(QString str, int gsize)
{
    QGroupBox_pointer = new QGroupBox(str); // Создание GroupBox
    QGroupBox_pointer->setFixedHeight(gsize);
    QGroupBox_pointer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Настройка размера
}

void oscillo_channel::ConnectSeries()
{
    ChartView_pointer.chart()->addSeries(&Series_pointer);
    ChartView_pointer.chart()->createDefaultAxes();
    ChartView_pointer.chart()->legend()->hide();
}
void oscillo_channel::DisconnectSeries()
{
    delete QGroupBox_pointer;
        // delete Series_pointer[arg1];
        // delete ChartView_pointer[arg1];
}
