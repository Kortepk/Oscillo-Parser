#ifndef OSCILLO_CHANNEL_H
#define OSCILLO_CHANNEL_H

#include <QGroupBox>
#include <QChart>
#include <QLineSeries>
#include <QChartView>

class oscillo_channel
{
public:
    oscillo_channel();

    void initSine();
    void ReplaceDots();
    void SetGraphAxis(float *min_x, float *min_y, float *max_x, float *max_y);
    void ClearPoints();
    void ChangeGroupSize(int val);
    void CreateGroupBox(QString str, int gsize);
    void ConnectSeries();
    void DisconnectSeries();

    QGroupBox *QGroupBox_pointer = nullptr; // Массив указателей на GroupBox
    QtCharts::QLineSeries Series_pointer; // Указатели на данные графика
    QtCharts::QChartView ChartView_pointer; // Массив графиков

    QList<QPointF> ListPoint; // Теневая переменная для буфферизации

};

#endif // OSCILLO_CHANNEL_H
