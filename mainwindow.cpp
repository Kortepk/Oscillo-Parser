#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCore/QtDebug>
#include <QPushButton>
#include <QChart>
#include <QLineSeries>
#include <QChartView>
#include <QMessageBox>
#include <QElapsedTimer>
#include "settingsdialog.h"
#include <QTimer>
#include "ControlPanel.h"
#include <QCloseEvent>
#include <QValueAxis>
#include <cmath>

#define byte5 0

QSpacerItem *refOscilloSpacer;
QGroupBox* QGroupBox_pointer[10]; // Массив указателей на GroupBox
QtCharts::QLineSeries* Series_pointer[10]; // Указатели на данные графика
QtCharts::QChartView* ChartView_pointer[10]; // Массив графиков

QList<QPointF> ListPoint[10]; // Теневая переменная для буфферизации

QSerialPort* MainPort = nullptr;

QByteArray RxBuffer;

QElapsedTimer Maintimer;
QPointF TempPoint;


auto start_time = std::chrono::high_resolution_clock::now();
QElapsedTimer MainTimer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new SettingsDialog(this)),
    SetDial(new SettingsDialog(this)),
    ControlPnl()
{
    ui->setupUi(this);

    initSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::initSine(int channel)
{
    Series_pointer[channel]->clear();

    float i;

    for(i=0; i <= M_PI * 6; i+=0.05)
        ListPoint[channel] << QPointF(i, 2.5 * sin(i));

    ListPoint[channel] << QPointF(i, 0); // Последнее значение

    Series_pointer[channel]->replace(ListPoint[channel]);

    qDebug() << "init" << channel;
}


void MainWindow::initSettings()
{
    MainPort = new QSerialPort(this);
    ControlPnl = new ControlPanel();

    ControlPnlDialog = new QDialog();
    ControlPnlDialog->setWindowTitle("Control panel");

    Maintimer.start();

    //** init **//
    initActionsConnections();

    Series_pointer[0] = new QtCharts::QLineSeries();
    ChartView_pointer[0] = new QtCharts::QChartView();

    initSine(0);

    ChartView_pointer[0]->chart()->legend()->hide();
    ChartView_pointer[0]->chart()->addSeries(Series_pointer[0]);
    ChartView_pointer[0]->chart()->createDefaultAxes();

    QVBoxLayout *groupBoxLayout = new QVBoxLayout();
    groupBoxLayout->addWidget(ChartView_pointer[0]); // Создаём текст внутри GroupBox

    delete ui->Channel1_groupBox->layout(); // Удаляем layout от дизайнера
    ui->Channel1_groupBox->setLayout(groupBoxLayout);

    refOscilloSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);

    QGroupBox_pointer[0] = ui->Channel1_groupBox; // Main oscillo GroupBox


    //ui->OscilloPanel_Layout->setContentsMargins(0, 0, 0, 0); // Установка нулевых отступов
    // Добавление виджета в макет окна
    ui->OscilloPanel_Layout->addWidget(ControlPnl);


    //dialog.show();
    // Отображение нового окна

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::UpdateGraph));
    timer->start(1000/75); // /75
}

void MainWindow::UpdateGraph(void)
{

    if(MainPort->isOpen() && (fillingIndex >= 2)) // 2 for trigger
    {
        for(int i = 0; i < Channel_Size; i++)
        {
            Series_pointer[i]->replace(ListPoint[i]);
        }
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->PortSettings_action, &QAction::triggered, SetDial, &SettingsDialog::show);
    connect(MainPort, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(ControlPnl, &ControlPanel::TurnFlowMode_Signal, this, &MainWindow::ChangeFlowWindowMode);
    connect(ControlPnl, &ControlPanel::GroupSize_Signal, this, &MainWindow::ChangeGroupSize);
    connect(ControlPnl, &ControlPanel::CounterChannel_Signal, this, &MainWindow::CounterChannel_Changed);
    connect(ControlPnl, &ControlPanel::ChannelChange_Signal, this, &MainWindow::ChangeGraph);
    connect(ControlPnl, &ControlPanel::StartPause_Signal, this, &MainWindow::StartPauseReadData);
    connect(ControlPnl, &ControlPanel::TestPushButton_Signal, this, &MainWindow::TestFunction);
    connect(ControlPnl, &ControlPanel::ChangeParseMode_Signal, this, &MainWindow::ChangeParsingMode);
    connect(ControlPnl, &ControlPanel::TriggerChanged_Signal, this, &MainWindow::TrigerValueChanged);
    connect(ControlPnl, &ControlPanel::ClickHalfTrig_Signal, this, &MainWindow::CalcHalfTrigger);
    connect(ControlPnl, &ControlPanel::AutoSize_Signal, this, &MainWindow::AutoSizeClick);

    connect(ControlPnlDialog, &QDialog::finished, this, &MainWindow::CloseFlowPanel);
}

void MainWindow::AutoSizeClick()
{
    if(LastMinPoint != LastMaxPoint) // Есть хоть какие-то значения
    {
        float HalfVal = (LastMaxPoint + LastMinPoint)/2;
        ControlPnl->SetDialPositionScale(22.5, HalfVal, 45, LastMaxPoint - LastMinPoint);
    }
}

void MainWindow::CalcHalfTrigger()
{
    float HalfVal = (LastMaxPoint + LastMinPoint)/2;
    ControlPnl->SetTrigValue(HalfVal);
}

void MainWindow::TrigerValueChanged(int channel, float val)
{
    TriggerValue = val;
#if 0
    Series_pointer[9] = new QtCharts::QLineSeries();

    ListPoint[9] << QPointF(0, val) << QPointF(100, val);

    Series_pointer[9]->replace(ListPoint[9]);

    Series_pointer[9]->setColor(QColor(255, 128, 0));
    ChartView_pointer[channel]->chart()->addSeries(Series_pointer[9]);
#endif
    qDebug() << val;
}

void MainWindow::ChangeParsingMode(int mode)
{
    TriggerMode = mode;

    if(TriggerMode == 1)
    {
        PortReadFlag = true;
        fillingIndex = 0;
    }
}

void MainWindow::TestFunction()
{
    for(int i=0; i < ListPoint[0].size(); i++)
        qDebug() << ListPoint[0].at(i);
}

void MainWindow::StartPauseReadData()
{
    PortReadFlag = !PortReadFlag;
}

void MainWindow::ConcreteChangeGraph(int *channel, float *min_x, float *min_y, float *max_x, float *max_y)
{
    QtCharts::QChart *chart = ChartView_pointer[*channel]->chart();

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
    Series_pointer[*channel]->attachAxis(axisX);
    Series_pointer[*channel]->attachAxis(axisY);
}

void MainWindow::ChangeGraph(int channel)//float shift_x, float shift_y, float scale_x, float scale_y)
{
    if(channel <= 0) // Если мы производим настройку по времени
    {   // (0.0; 1.0)  * (max_x - min_x)
        ControlPnl->ViewGraphSet.ShiftMid_x = ControlPnl->ViewGraphSet.GraphShiftX + 10 * ControlPnl->ViewGraphSet.DialTurnoversX;
    }

    float min_x = ControlPnl->ViewGraphSet.ShiftMid_x - (ControlPnl->ViewGraphSet.GraphScaleX * ControlPnl->ViewGraphSet.ScalePrefixX)/2,
          min_y = 0,
          max_x = ControlPnl->ViewGraphSet.ShiftMid_x + (ControlPnl->ViewGraphSet.GraphScaleX * ControlPnl->ViewGraphSet.ScalePrefixX)/2,
          max_y = 0;


    if(channel > 0) // Если дан конкретный канал
    {
        float shift_y = ControlPnl->ViewGraphSet.ChannelShiftY + 10 * ControlPnl->ViewGraphSet.DialTurnoversY;
        min_y = shift_y - ControlPnl->ViewGraphSet.ChannelScaleY/2;
        max_y = shift_y + ControlPnl->ViewGraphSet.ChannelScaleY/2;
        channel -= 1;
        ConcreteChangeGraph(&channel, &min_x, &min_y, &max_x, &max_y);
    }
    else
        for(int i = 0; i < Channel_Size; i ++)
        {
            QtCharts::QAbstractAxis *oldAxisY = ChartView_pointer[i]->chart()->axes(Qt::Vertical).at(0);

            QtCharts::QValueAxis  * axisY = static_cast<QtCharts::QValueAxis * >(oldAxisY);
            min_y = axisY->min();
            max_y = axisY->max();

            ConcreteChangeGraph(&i, &min_x, &min_y, &max_x, &max_y);
        }
}

void MainWindow::CloseFlowPanel()
{
    //qDebug() << __PRETTY_FUNCTION__ ;
    ControlPnl->Change_TurnFlowMode(false);
}

void MainWindow::ChangeFlowWindowMode(bool TriggerStatus)
{
    if(TriggerStatus)
    {
        ui->OscilloPanel_Layout->removeWidget(ControlPnl);

        QVBoxLayout layout(ControlPnlDialog);
        layout.setContentsMargins(0, 0, 0, 0); // Установка нулевых отступов
        layout.addWidget(ControlPnl);  // Добавление виджета в макет окна

        ControlPnlDialog->show();
    }
    else
    {
        ControlPnlDialog->hide();
        ui->OscilloPanel_Layout->addWidget(ControlPnl);
    }
}

void MainWindow::readData()
{
    if(!PortReadFlag && fillingIndex == 0)
    {
        MainPort->clear();
        MainTimer.restart();
        return;
    }

    RxBuffer += MainPort->readAll();

    while((RxBuffer[0] != ';') && (RxBuffer.length() > 0)) // Условие синхронизации, относительно последнего числа и символа ;
    {
        RxBuffer.remove(0, 1);
    }

    int indexEOF = 0, indexSOF = 0;
    QString str;
    const int MaxPoint = ControlPnl->Get_MaxPointSlider();

    while(indexSOF < RxBuffer.length())
    {
        indexSOF = indexEOF;
        indexEOF = RxBuffer.indexOf(';', indexEOF + 1); // Пропускаем [0] символ ';'
        if (indexEOF != -1) {
            if(indexEOF - indexSOF > 2) // [1] - номер канала; [2] - ')'
            {
                // TODO handler a specific channel
                int NumberChannel = RxBuffer[indexSOF + 1] - '0';
                str = "";
                indexSOF += 3;
                while(indexSOF < indexEOF)
                    str += RxBuffer[indexSOF++];

                if(NumberChannel == 1)
                {
                    float ReadingValue = str.toFloat();

                    if(fillingIndex == 0)
                        MainTimer.restart();
                    else
                        if((TriggerMode >= 1) && (fillingIndex <= 1)) // Only for fillingIndex == 1
                    {
                        if((ListPoint[0].at(0).y() <= TriggerValue) && (TriggerValue <= ReadingValue))
                        {
                            ; // ControlPnl->on_StartPause_Button_clicked(); // Выключаем приём
                        }
                        else
                        {
                            MainTimer.restart();
                            fillingIndex = 0;
                        }
                    }


                    if(ListPoint[0].size() < MaxPoint)
                    {
                        ListPoint[0] << QPointF(MainTimer.nsecsElapsed() * 0.000001, ReadingValue); //
                        fillingIndex ++;

                        if(fillingIndex >= MaxPoint)
                        {
                            MainTimer.restart();
                            fillingIndex = 0;
                        }
                    }
                    else
                    {
                        if(ListPoint[0].size() != MaxPoint) // Массив стал меньше
                        {
                            while(ListPoint[0].size() != MaxPoint)
                                ListPoint[0].removeLast();

                            if(fillingIndex >= ListPoint[0].size())
                            {
                                MainTimer.restart();
                                fillingIndex = 0;
                            }
                        }

                        TempPoint = ListPoint[0].at(fillingIndex);
                        TempPoint.setX(MainTimer.nsecsElapsed() * 0.000001);
                        TempPoint.setY(ReadingValue);
                        ListPoint[0].replace(fillingIndex, TempPoint);

                        fillingIndex ++;

                        if(fillingIndex >= MaxPoint)
                        {
                            MainTimer.restart();
                            fillingIndex = 0;

                            if(TriggerMode == 1)
                                ControlPnl->on_StartPause_Button_clicked();
                            // Nothing for TriggerMode == 2

                            LastMinPoint = NowMinPoint;
                            LastMaxPoint = NowMaxPoint;
                        }
                    }

                    if(fillingIndex == 1)
                    {
                        NowMinPoint = ReadingValue;
                        NowMaxPoint = ReadingValue;
                    }

                    if(ReadingValue < NowMinPoint)
                        NowMinPoint = ReadingValue;

                    if(ReadingValue > NowMaxPoint)
                        NowMaxPoint = ReadingValue;

                    //qDebug() << fillingIndex << str.toFloat();

                }
                if((NumberChannel == 3) && (Channel_Size > 1))
                {
                    if(ListPoint[1].size() <= MaxPoint)
                        ListPoint[1] << QPointF(MainTimer.elapsed() * 0.001, str.toFloat()); //
                    else
                    {
                        if(ListPoint[0].size() != MaxPoint) // Массив стал меньше
                        {
                            while(ListPoint[0].size() != MaxPoint)
                                ListPoint[0].removeLast();
                        }

                        TempPoint = ListPoint[1].at(fillingIndex);
                        TempPoint.setY(str.toFloat());
                        TempPoint.setY(str.toFloat());
                        ListPoint[1].replace(fillingIndex, TempPoint);
                    }
                }
            }
            RxBuffer.remove(0, indexEOF);
        }
        else
            break;

#if byte5
        if(ListPoint[0].size() <= ui->MaxPointSlider->value())
            ListPoint[0] << QPointF(ListPoint[0].size(), RxBuffer[indexSOF]); //
        else
        {
            TempPoint = ListPoint[0].at(fillingIndex);
            TempPoint.setY(RxBuffer[indexSOF]);
            ListPoint[0].replace(fillingIndex, TempPoint);
        }
        indexSOF ++;
#endif
    }
}

void MainWindow::ChangeGroupSize(int val)
{
    for(int i=0; i < Channel_Size; i++)
    {
        QGroupBox_pointer[i]->setMinimumSize(0, val);
        QGroupBox_pointer[i]->setFixedHeight(val);
    }
}

void MainWindow::CounterChannel_Changed(int arg1)
{
    qDebug() << __PRETTY_FUNCTION__ << arg1;

    if(Channel_Size < arg1) // Количество каналов увеличилось
    {
        const int chn = Channel_Size;
        QGroupBox_pointer[chn] = new QGroupBox("Channel " + QString::number(arg1)); // Создание GroupBox
        QGroupBox_pointer[chn]->setFixedHeight( ControlPnl->Get_GroupSizeValue() );
        QGroupBox_pointer[chn]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Настройка размера

        Series_pointer[chn] = new QtCharts::QLineSeries();  // Create object and get this pointter
        ChartView_pointer[chn] = new QtCharts::QChartView();

        if(!MainPort->isOpen())
            initSine(chn);

        ChartView_pointer[chn]->chart()->addSeries(Series_pointer[chn]);
        ChartView_pointer[chn]->chart()->createDefaultAxes();
        ChartView_pointer[chn]->chart()->legend()->hide();

        // QLabel *label = new QLabel("Label inside Group Box");
        QVBoxLayout *groupBoxLayout = new QVBoxLayout();
        groupBoxLayout->addWidget(ChartView_pointer[chn]); // Создаём текст внутри GroupBox
        QGroupBox_pointer[chn]->setLayout(groupBoxLayout);

        ui->Oscillo_Channel_Area_verticalLayout->addWidget(QGroupBox_pointer[chn]); // Добавляем GB

        ui->Oscillo_Channel_Area_verticalLayout->removeItem(refOscilloSpacer);
        ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);
    }
    else
    {
        //ui->Oscillo_Channel_Area_verticalLayout->removeWidget(QGroupBox_pointer[arg1]);

        delete QGroupBox_pointer[arg1];
        // delete Series_pointer[arg1];
        // delete ChartView_pointer[arg1];
    }
    Channel_Size = arg1;
}



void MainWindow::on_Connect_action_triggered()
{
    if(!MainPort->isOpen())
    {
        const SettingsDialog::Settings p = m_settings->settings();
        MainPort->setPortName(p.name);
        MainPort->setBaudRate(p.baudRate);
        MainPort->setDataBits(p.dataBits);
        MainPort->setParity(p.parity);
        MainPort->setStopBits(p.stopBits);
        MainPort->setFlowControl(p.flowControl);
        if (MainPort->open(QIODevice::ReadWrite)) {
            qDebug() << (tr("Connected to %1 : %2, %3, %4, %5, %6")
                                    .arg(p.name, p.stringBaudRate, p.stringDataBits,
                                    p.stringParity, p.stringStopBits, p.stringFlowControl));
        } else {
            QMessageBox::critical(this, tr("Error"), MainPort->errorString());
            return;
        }

        for(int i=0;i<Channel_Size;i++){

            Series_pointer[i]->clear();
            ListPoint[i].clear();
        }

        if(!PortReadFlag)
        {
            ControlPnl->on_StartPause_Button_clicked(); // При подключении включаем приём
        }

        MainPort->clear();
        MainTimer.restart();
        ui->Connect_action->setText("Disonnect");
    }
    else
    {
        MainPort->close();
        ui->Connect_action->setText("Connect");
    }

#if 0
    const SettingsDialog::Settings settings = SetDial->settings();
    qDebug() << "Name:" << settings.name;
    qDebug() << "Baud Rate:" << settings.baudRate << "(" << settings.stringBaudRate << ")";
    qDebug() << "Data Bits:" << settings.dataBits << "(" << settings.stringDataBits << ")";
    qDebug() << "Parity:" << settings.parity << "(" << settings.stringParity << ")";
    qDebug() << "Stop Bits:" << settings.stopBits << "(" << settings.stringStopBits << ")";
    qDebug() << "Flow Control:" << settings.flowControl << "(" << settings.stringFlowControl << ")";
#endif
}


void MainWindow::on_PortSettings_action_triggered()
{
    SetDial->LoadSettings();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if(SetDial != nullptr)
        delete SetDial;
    if(ControlPnlDialog != nullptr)
        delete ControlPnlDialog;
}
