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

int fillingIndex = 0;// Заполняемый индекс

auto start_time = std::chrono::high_resolution_clock::now();

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

    Series_pointer[0]->append(0, -2);
    Series_pointer[0]->append(1000, 2);

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

    if(MainPort->isOpen())
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

    connect(ControlPnlDialog, &QDialog::finished, this, &MainWindow::CloseFlowPanel);
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

    qDebug() << QString::number(*channel) + ")" << *min_x << *min_y << *max_x << *max_y;

}

void MainWindow::ChangeGraph(int channel, float shift_x, float shift_y, float scale_x, float scale_y)
{
    qDebug() << shift_x << shift_y << scale_x << scale_y;
    float min_x = shift_x - scale_x/2, min_y = shift_y - scale_y/2, max_x = shift_x + scale_x/2, max_y = shift_y + scale_y/2;
    if(channel >= 0) // Если дан конкретный канал
    {
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
                    if(ListPoint[0].size() <= MaxPoint)
                        ListPoint[0] << QPointF(ListPoint[0].size(), str.toFloat()); //
                    else
                    {
                        TempPoint = ListPoint[0].at(fillingIndex);
                        TempPoint.setY(str.toFloat());
                        ListPoint[0].replace(fillingIndex, TempPoint);

                        fillingIndex ++;
                        if(fillingIndex >= ListPoint[0].size())
                            fillingIndex = 0;
                    }
                    //qDebug() << fillingIndex << str.toFloat();

                }
                if((NumberChannel == 3) && (Channel_Size > 1))
                {
                    if(ListPoint[1].size() <= MaxPoint)
                        ListPoint[1] << QPointF(ListPoint[1].size(), str.toFloat()); //
                    else
                    {
                        TempPoint = ListPoint[1].at(fillingIndex);
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

        Series_pointer[chn]->append(0, -2);
        Series_pointer[chn]->append(1000, 2);
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
        MainPort->flush();
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


void MainWindow::on_MaxPointSlider_sliderMoved(int position)
{
    ;//qDebug() << position;

}


void MainWindow::on_pushButton_clicked()
{
    for(int i=0; i < ListPoint[0].size(); i++)
    {
        qDebug() << ListPoint[0].at(i).x() << ListPoint[0].at(i).y();
    }
}


void MainWindow::on_pushButtonTest_clicked()
{
    QDialog dialog;
    dialog.setWindowTitle("New Window");

    QVBoxLayout layout(&dialog);
    layout.setContentsMargins(0, 0, 0, 0); // Установка нулевых отступов

    // Добавление виджета в макет окна
    ControlPanel frm;
    layout.addWidget(&frm);

    //dialog.show();
    // Отображение нового окна
    dialog.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(SetDial != nullptr)
        delete SetDial;
    if(ControlPnlDialog != nullptr)
        delete ControlPnlDialog;
}
