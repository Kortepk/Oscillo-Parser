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

QSpacerItem *refOscilloSpacer;
QGroupBox* QGroupBox_pointer[10]; // Массив указателей на GroupBox
QtCharts::QLineSeries* Series_pointer[10]; // Указатели на данные графика
QtCharts::QChartView* ChartView_pointer[10]; // Массив графиков

QList<QPointF> ListPoint[10]; // Теневая переменная для буфферизации

QSerialPort* MainPort = nullptr;

QByteArray RxBuffer;

QElapsedTimer Maintimer;
QPointF TempPoint;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings(new SettingsDialog(this)),
    SetDial(new SettingsDialog(this))
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

    Maintimer.start();

    //** init **//
    initActionsConnections();

    Series_pointer[0] = new QtCharts::QLineSeries();
    ChartView_pointer[0] = new QtCharts::QChartView();

    Series_pointer[0]->append(0, -2);
    Series_pointer[0]->append(10000, 2);

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

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::UpdateGraph));
    timer->start(100); // /75
}

void MainWindow::UpdateGraph(void)
{
    Series_pointer[0]->clear();
    //*Series_pointer[0] << ListPoint[0];
    Series_pointer[0]->replace(ListPoint[0]);

    //qDebug() << Series_pointer[0] ->points().size();
    return;
    if(MainPort->isOpen())
    {

        float tm = TempPoint.y();
        Series_pointer[0]->replace(0, TempPoint);

        TempPoint = Series_pointer[0]->points().at(1);
        TempPoint.setY(tm);

        Series_pointer[0]->replace(1, TempPoint);
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->PortSettings_action, &QAction::triggered, SetDial, &SettingsDialog::show);
    connect(MainPort, &QSerialPort::readyRead, this, &MainWindow::readData);
}

void MainWindow::readData()
{
    RxBuffer += MainPort->readAll();

    while((RxBuffer[0] != ';') && (RxBuffer.length() > 0)) // Условие синхронизации, относительно последнего числа и символа ;
    {
        RxBuffer.remove(0, 1);
    }
    //qDebug() << MainPort->bytesAvailable();

    int indexEOF = 0, indexSOF = 0;
    QString str;
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
                    //Series_pointer[0]->append(Maintimer.elapsed(), str.toFloat());
                    if(ListPoint[0].size() > ui->MaxPointSlider->value())
                        ListPoint[0].clear();  // Maybe need Rewrite
                    ListPoint[0] << QPointF(ListPoint[0].size(), str.toFloat());
                    //qDebug() << ListPoint[0].size() << str.toFloat();
#if 0
                    TempPoint = Series_pointer[0]->points().at(0);
                    TempPoint.setY(str.toFloat());  // Установка нового значения y для первой точки
                    Series_pointer[0]->replace(0, TempPoint);
#endif

                }
                if((NumberChannel == 3) && (Channel_Size > 1))
                {
                    //Series_pointer[1]->append(Maintimer.elapsed(), str.toFloat());
                    //*Series_pointer[1] << QPointF(Maintimer.elapsed(), str.toFloat());
#if 0
                    TempPoint = Series_pointer[1]->points().at(0);
                    TempPoint.setY(str.toFloat());  // Установка нового значения y для первой точки
                    Series_pointer[1]->replace(0, TempPoint);
#endif
                }
            }
        }
        else
            break;
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

void MainWindow::on_Group_Size_Slider_sliderMoved(int position)
{
    qDebug() << position<< "\n";

    ui->Group_Size_Box->setValue(position);

    //ui->Channel1_groupBox->setMinimumSize(0, position);]
    ChangeGroupSize(position);
}



void MainWindow::on_Group_Size_Box_valueChanged(int arg1)
{
    if(arg1 <= 1000)
        ui->Group_Size_Slider->setValue(arg1);
    else
        if(ui->Group_Size_Slider->value() != 1000)
            ui->Group_Size_Slider->setValue(1000);

    ChangeGroupSize(arg1);
}



void MainWindow::on_Counter_channel_Box_valueChanged(int arg1)
{
    if(Channel_Size < arg1) // Количество каналов увеличилось
    {
        const int chn = Channel_Size;
        QGroupBox_pointer[chn] = new QGroupBox("Channel " + QString::number(arg1)); // Создание GroupBox
        QGroupBox_pointer[chn]->setFixedHeight(ui->Group_Size_Box->value());
        QGroupBox_pointer[chn]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Настройка размера

        Series_pointer[chn] = new QtCharts::QLineSeries();  // Create object and get this pointter
        ChartView_pointer[chn] = new QtCharts::QChartView();

        Series_pointer[chn]->append(0, -2);
        Series_pointer[chn]->append(2, 2);
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
        delete Series_pointer[arg1];
        delete ChartView_pointer[arg1];
        qDebug() << "Ok";
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
    qDebug() << position;
}

