#ifndef OSCILLO_CHANNEL_H
#define OSCILLO_CHANNEL_H

#include <QObject>
#include <QGroupBox>
#include <QChart>
#include <QLineSeries>
#include <QChartView>

class oscillo_channel : public QObject
{
    Q_OBJECT
public:
    oscillo_channel(QObject *parent = nullptr);

    void initSine();
    void ReplaceDots();
    void SetGraphAxis(float *min_x, float *min_y, float *max_x, float *max_y);
    void ClearPoints();
    void ChangeGroupSize(int val);
    void CreateGroupBox(QString str, int gsize);
    void ConnectSeries();
    void DisconnectSeries();
    void ValueProcessing(float ReadingValue, float DeltaTime);
    void SaveLastValue();

    QGroupBox *QGroupBox_pointer = nullptr; // Массив указателей на GroupBox
    QtCharts::QChartView ChartView_pointer; // Массив графиков
    int MaxPoint = 1000;
    int fillingIndex = 0;     // Если в канале индекс выйдет за границы, то мы должно синхронизировать и другие каналы, сообщив это
    int TriggerValue = 0;     // Число, по которому срабатывает триггер
    bool AddPointFlag = true; // Флаг, разрешающий приём
    float LastTime = 0;       // Запомннает предыдущее время (положение по x)
    bool ModeMaster = false;  // Определяет кем будет какнал 0 - slaves; 1 - master. Не допускается два мастера!

    float LastMinPoint = 0; // Запомниает максимальную и минимальную точку на всём графике
    float LastMaxPoint = 0;
    float LastMaxTime = 0;

signals:
    void MasterHandle_Signal(bool status); // Master сообщает slaves о необходимости начать/прекратить сбор данных
    void OverloadPoints();                 // Сообщает всем остальным, что произошло переполнение буффера, и необходима синхронизация

private:
    QtCharts::QLineSeries Series_pointer; // Указатели на данные графика
    QList<QPointF> ListPoint; // Теневая переменная для буфферизации
    QPointF TempPoint;

    float NowMinPoint, NowMaxPoint;

};

#endif // OSCILLO_CHANNEL_H
