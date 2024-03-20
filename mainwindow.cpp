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

QSpacerItem *refOscilloSpacer;
QGroupBox* QGroupBox_pointer[10]; // Массив указателей
QSerialPort* MainPort = nullptr;

QByteArray RxBuffer;
QtCharts::QLineSeries* MainSeries = nullptr;
QtCharts::QChartView* chartView = nullptr;
QElapsedTimer Maintimer;

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
    MainSeries = new QtCharts::QLineSeries();
    chartView = new QtCharts::QChartView();

    MainSeries->setName("Hello");
    chartView->chart()->addSeries(MainSeries);
    //chartView->chart()->createDefaultAxes();

    QVBoxLayout *groupBoxLayout = new QVBoxLayout();
    groupBoxLayout->addWidget(chartView); // Создаём текст внутри GroupBox

    delete ui->Channel1_groupBox->layout(); // Удаляем layout от дизайнера
    ui->Channel1_groupBox->setLayout(groupBoxLayout);

    refOscilloSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);

    QGroupBox_pointer[0] = ui->Channel1_groupBox; // Main oscillo GroupBox
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
                    //qDebug() << str;
                    MainSeries->append(Maintimer.elapsed(), str.toFloat());
                    chartView->chart()->removeSeries(MainSeries);
                    chartView->chart()->createDefaultAxes();
                    chartView->chart()->addSeries(MainSeries);
                    //chartView->repaint();
                }
            }
        }
        else
            break;
    }
}

void MainWindow::ChangeGroupSize(int val)
{
    for(int i=0; i < Last_Num_Channel; i++)
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
    if(Last_Num_Channel < arg1) // Количество каналов увеличилось
    {
        QGroupBox_pointer[Last_Num_Channel] = new QGroupBox("Channel " + QString::number(arg1)); // Создание GroupBox
        QGroupBox_pointer[Last_Num_Channel]->setFixedHeight(ui->Group_Size_Box->value());
        QGroupBox_pointer[Last_Num_Channel]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed); // Настройка размера

        QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
        QtCharts::QChartView* chartView = new QtCharts::QChartView();
        series->append(0, 6);
        series->append(2, 4);
        chartView->chart()->addSeries(series);
        chartView->chart()->createDefaultAxes();

        // QLabel *label = new QLabel("Label inside Group Box");
        QVBoxLayout *groupBoxLayout = new QVBoxLayout();
        groupBoxLayout->addWidget(chartView); // Создаём текст внутри GroupBox
        QGroupBox_pointer[Last_Num_Channel]->setLayout(groupBoxLayout);

        ui->Oscillo_Channel_Area_verticalLayout->addWidget(QGroupBox_pointer[Last_Num_Channel]); // Добавляем GB

        ui->Oscillo_Channel_Area_verticalLayout->removeItem(refOscilloSpacer);
        ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);
    }
    else
    {
        //ui->Oscillo_Channel_Area_verticalLayout->removeWidget(QGroupBox_pointer[arg1]);
        delete QGroupBox_pointer[arg1];
        qDebug() << "Ok";
    }
    Last_Num_Channel = arg1;
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

