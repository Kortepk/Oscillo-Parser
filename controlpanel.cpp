#include "controlpanel.h"
#include "qelapsedtimer.h"
#include "ui_controlpanel.h"
#include <QDebug>
#include <QTimer>
#include <cmath>

bool TurnFlowMode_Flag = false;

QElapsedTimer TimerKnobClick;

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    // Настройка комбобоксов
    //for(int i=1; i<=9; i++)
    //    ui->ChannelSelection_comboBox->addItem("Channel " + QString::number(i), i);

    ui->ChannelSelection_comboBox->addItem("Channel 1", 1);
    //ui->PrefixScaleY_comboBox->setCurrentIndex(0);

    ui->PrefixScaleX_comboBox->addItem(tr("kilo"), 3);
    ui->PrefixScaleX_comboBox->addItem(tr("hecto"), 2);
    ui->PrefixScaleX_comboBox->addItem(tr("deca"), 1);
    ui->PrefixScaleX_comboBox->addItem(tr("^0"), 0);
    ui->PrefixScaleX_comboBox->addItem(tr("deci"), -1);
    ui->PrefixScaleX_comboBox->addItem(tr("centi"), -2);
    ui->PrefixScaleX_comboBox->addItem(tr("milli"), -3);
    ui->PrefixScaleX_comboBox->addItem(tr("^-4"), -4);
    ui->PrefixScaleX_comboBox->addItem(tr("^-5"), -5);
    ui->PrefixScaleX_comboBox->addItem(tr("micro"), -6);
    ui->PrefixScaleX_comboBox->setCurrentIndex(1);
/*
    ui->PrefixScaleY_comboBox->addItem(tr("kilo"), 3);
    ui->PrefixScaleY_comboBox->addItem(tr("^0"), 0);
    ui->PrefixScaleY_comboBox->addItem(tr("milli"), -3);
    ui->PrefixScaleY_comboBox->addItem(tr("micro"), -6);
    ui->PrefixScaleY_comboBox->addItem(tr("nano"), -9);
    ui->PrefixScaleY_comboBox->setCurrentIndex(1);
*/
    QTreeWidgetItem *itm = new QTreeWidgetItem(ui->treeWidget);
    QTreeWidgetItem *itm2 = new QTreeWidgetItem(itm);
    itm->setText(0, "Hello");

    itm2->setText(0, "World");
    ui->treeWidget->addTopLevelItem(itm2);

    connect(UItimer, &QTimer::timeout, this, QOverload<>::of(&ControlPanel::CheckUI));
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::CheckUI() // Использую, что бы при двойном клике на крутилку её заблокировать
{
    if(KnobBlockFlag)
    {
        ui->ChannalPosition_dial->setEnabled(true);
        ui->GraphPosition_dial->setEnabled(true);
        ui->ChannalScale_dial->setEnabled(true);
        ui->GraphScale_dial->setEnabled(true);

        UItimer->stop();
        KnobBlockFlag = false;
    }
}

void ControlPanel::CheckTurnFlowMode()
{
    if(TurnFlowMode_Flag) // Оконный\плавающий режим панели
    {
        ui->TurnFlowMode->setText("▲");
    }
    else
    {
        ui->TurnFlowMode->setText("▼");
    }

    emit TurnFlowMode_Signal(TurnFlowMode_Flag);
}

void ControlPanel::on_TurnFlowMode_clicked()
{
    TurnFlowMode_Flag = !TurnFlowMode_Flag;

    CheckTurnFlowMode();
}

void ControlPanel::Change_TurnFlowMode(bool FlowMode_Flag)
{
    TurnFlowMode_Flag = FlowMode_Flag;

    CheckTurnFlowMode();
}

int ControlPanel::Get_MaxPointSlider()
{
    return ui->MaxPoint_Slider->value();
}

int ControlPanel::Get_GroupSizeValue()
{
    return ui->GroupSize_Box->value();
}


void ControlPanel::on_GroupSize_Slider_sliderMoved(int position)
{
    ui->GroupSize_Box->setValue(position);

    emit GroupSize_Signal(position);
}


void ControlPanel::on_GroupSize_Box_valueChanged(int arg1)
{

    if(arg1 <= 1000)
        ui->GroupSize_Slider->setValue(arg1);
    else
        if(ui->GroupSize_Slider->value() != 1000)
            ui->GroupSize_Slider->setValue(1000);

    emit GroupSize_Signal(arg1);
}


void ControlPanel::on_CounterChannel_Box_valueChanged(int arg1)
{
    if(ChannalSize < arg1)
    {
        ChannalSize = arg1;
        ui->ChannelSelection_comboBox->addItem("Channel " + QString::number(ChannalSize), ChannalSize);
    }
    else
    {
        ChannalSize = arg1;
        ui->ChannelSelection_comboBox->removeItem(ChannalSize);
    }

    emit CounterChannel_Signal(arg1);
}

void ControlPanel::on_GraphPosition_dial_valueChanged(int value)
{
    ViewGraphSet.GraphShiftX = value;
    emit ChannelChange_Signal(-1);
}

void ControlPanel::on_GraphScale_dial_valueChanged(int value)
{
    float val_fl = value / 100.f;

    qDebug() << val_fl;

    if(ViewGraphSet.GraphScaleX > 9.9 && val_fl < 1.1) // Увеличить степень
    {
        if(ui->PrefixScaleX_comboBox->currentIndex() > 0)
            ui->PrefixScaleX_comboBox->setCurrentIndex(ui->PrefixScaleX_comboBox->currentIndex()-1);
    }
    else
    if(ViewGraphSet.GraphScaleX < 1.1 && val_fl > 9.9) // Уменьшить степень
    {
        if(ui->PrefixScaleX_comboBox->currentIndex() < 9)
            ui->PrefixScaleX_comboBox->setCurrentIndex(ui->PrefixScaleX_comboBox->currentIndex()+1);
    }

    ViewGraphSet.ScalePrefixX = pow(10, ui->PrefixScaleX_comboBox->currentData().value<int>());

    ViewGraphSet.GraphScaleX = val_fl;
    ui->GraphScale_Box->setValue(val_fl);

    emit ChannelChange_Signal(-1);
}



void ControlPanel::on_ChannalPosition_dial_valueChanged(int value)
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    ViewGraphSet.ChannelShiftY = value;
    emit ChannelChange_Signal(chan_num);
}


void ControlPanel::on_ChannalScale_dial_valueChanged(int value)
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    ViewGraphSet.ChannelScaleY = value/100.f;
    emit ChannelChange_Signal(chan_num);
}


void ControlPanel::on_StartPause_Button_clicked()
{
    emit StartPause_Signal();

    // Обратной связи от MainWindow нету, поэтмоу такое изменение кнопки

    if(ui->StartPause_Button->text() == "Pause")
    {
        ui->StartPause_Button->setText("Start");
        ui->StartPause_Button->setStyleSheet(
            "QPushButton{border-radius: 4px;background-color: rgb(156, 255, 156);}"
            );
    }
    else
    {
        ui->StartPause_Button->setText("Pause");
        ui->StartPause_Button->setStyleSheet(
            "QPushButton{border-radius: 4px;background-color: rgb(255, 156, 156);}"
            );
    }
}

/* SLIDERS_PRESSED() */

void ControlPanel::on_ChannalPosition_dial_sliderPressed()
{
    if(TimerKnobClick.elapsed() < 300)
    {
        ui->ChannalPosition_dial->setValue(500);
        ui->ChannalPosition_dial->setEnabled(false);
        KnobBlockFlag = true;
        UItimer->start(100);
    }
    TimerKnobClick.restart();
}


void ControlPanel::on_GraphPosition_dial_sliderPressed()
{
    if(TimerKnobClick.elapsed() < 300)
    {
        ui->GraphPosition_dial->setValue(500);
        ui->GraphPosition_dial->setEnabled(false);
        KnobBlockFlag = true;
        UItimer->start(100);
    }
    TimerKnobClick.restart();
}


void ControlPanel::on_ChannalScale_dial_sliderPressed()
{
    if(TimerKnobClick.elapsed() < 300)
    {
        ui->ChannalScale_dial->setValue(1);
        ui->ChannalScale_dial->setEnabled(false);
        KnobBlockFlag = true;
        UItimer->start(100);
    }
    TimerKnobClick.restart();
}


void ControlPanel::on_GraphScale_dial_sliderPressed()
{
    if(TimerKnobClick.elapsed() < 300)
    {
        ui->GraphScale_dial->setValue(1);
        ui->GraphScale_dial->setEnabled(false);
        KnobBlockFlag = true;
        UItimer->start(100);
    }
    TimerKnobClick.restart();
}


void ControlPanel::on_MaxPoint_Slider_sliderMoved(int position)
{
    ui->MaxPoint_Box->setValue(position);
}


void ControlPanel::on_pushButtonTest_clicked()
{
    emit TestPushButton_Signal();
}



