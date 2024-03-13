#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtCore/QtDebug>
#include <QPushButton>

int Last_Num_Channel = 1;
QSpacerItem *refOscilloSpacer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //** init **//
    refOscilloSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);

    QSerialPortInfo::availablePorts();
    //**      **//

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        qDebug() << "Name : " << info.portName();
        qDebug() << "Description : " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
    }

#if 0
    QGridLayout  * layout = new QGridLayout;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 7; ++j) {
            QPushButton  * button = new QPushButton(QString::number(i) + "x" + QString::number(j));
            layout->addWidget(button, i, j);
        }

    ui->scrollArea->setLayout(layout);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Group_Size_Slider_sliderMoved(int position)
{
    qDebug() << position<< "\n";

    ui->Group_Size_Box->setValue(position);

    ui->Channel1_groupBox->setMinimumSize(0, position);
}


void MainWindow::on_Group_Size_Box_valueChanged(int arg1)
{
    ui->Channel1_groupBox->setMinimumSize(0, arg1);
    if(arg1 <= 1000)
        ui->Group_Size_Slider->setValue(arg1);
    else
    {
        if(ui->Group_Size_Slider->value() != 1000)
            ui->Group_Size_Slider->setValue(1000);
    }
}


void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if(Last_Num_Channel < arg1) // Количество каналов увеличилось
    {
        QGroupBox *groupBox = new QGroupBox("My Group Box"); // Создание GroupBox
        groupBox->setFixedHeight(100);
        groupBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding); // Настройка размера

        QLabel *label = new QLabel("Label inside Group Box");
        QVBoxLayout *groupBoxLayout = new QVBoxLayout();
        groupBoxLayout->addWidget(label); // Создаём текст внутри GroupBox
        groupBox->setLayout(groupBoxLayout);

        ui->Oscillo_Channel_Area_verticalLayout->addWidget(groupBox); // Добавляем GB

        ui->Oscillo_Channel_Area_verticalLayout->removeItem(refOscilloSpacer);
        ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);
    }
    else
    {
        // TODO удалить лишние каналы
    }
    Last_Num_Channel = arg1;
}

