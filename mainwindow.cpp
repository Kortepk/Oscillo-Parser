#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCore/QtDebug>
#include <QPushButton>
#include <QChart>
#include <QLineSeries>
#include <QChartView>

#include "settingsdialog.h"

int Last_Num_Channel = 1;
QSpacerItem *refOscilloSpacer;

QGroupBox* QGroupBox_pointer[10]; // Массив указателей

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
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
    //** init **//
    initActionsConnections();

    QtCharts::QLineSeries* series = new QtCharts::QLineSeries();
    QtCharts::QChartView* chartView = new QtCharts::QChartView();
    series->append(0, 6);
    series->append(2, 4);
    series->setName("Hello");
    chartView->chart()->addSeries(series);
    chartView->chart()->createDefaultAxes();

    QVBoxLayout *groupBoxLayout = new QVBoxLayout();
    groupBoxLayout->addWidget(chartView); // Создаём текст внутри GroupBox
    delete ui->Channel1_groupBox->layout();
    ui->Channel1_groupBox->setLayout(groupBoxLayout);

    refOscilloSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);

    QGroupBox_pointer[0] = ui->Channel1_groupBox; // Main oscillo GroupBox
}

void MainWindow::initActionsConnections()
{
    connect(ui->PortSettings_action, &QAction::triggered, SetDial, &SettingsDialog::show);
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
        // TODO удалить лишние каналы
    }
    Last_Num_Channel = arg1;
}



void MainWindow::on_Connect_action_triggered()
{

}


void MainWindow::on_PortSettings_action_triggered()
{
    qDebug() << "Show";
}

