#include "controlpanel.h"
#include "qelapsedtimer.h"
#include "ui_controlpanel.h"
#include <QDebug>
#include <QTimer>
#include <cmath>
#include <QColorDialog>
#include <QString>

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

    ui->ChannelSelection_comboBox->addItem("Channel mix", 0);
    //ui->TrigChannel_comboBox->addItem("Channel mix", 0);

    ui->ChannelSelection_comboBox->setCurrentIndex(0);
    ui->TrigChannel_comboBox->setCurrentIndex(0);
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
    ui->PrefixScaleX_comboBox->setCurrentIndex(3);
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
        ui->TrigChannel_comboBox->addItem("Channel " + QString::number(ChannalSize), ChannalSize);
    }
    else
    {
        ChannalSize = arg1;
        ui->ChannelSelection_comboBox->removeItem(ChannalSize);
        ui->TrigChannel_comboBox->removeItem(ChannalSize);
    }

    emit CounterChannel_Signal(arg1);
}

void ControlPanel::on_GraphPosition_dial_valueChanged(int value)
{
    float val_fl = value / 100.f;

    if(ViewGraphSet.GraphShiftX > 9.5 && val_fl < 1.5) // Увеличить степень
    {
        ViewGraphSet.DialTurnoversX ++;
    }
    else
        if(ViewGraphSet.GraphShiftX < 1.5 && val_fl > 9.5) // Уменьшить степень
        {
            ViewGraphSet.DialTurnoversX --;
        }

    ViewGraphSet.GraphShiftX = val_fl;

    emit ChannelChange_Signal(-1);
}

void ControlPanel::on_GraphScale_dial_valueChanged(int value)
{
    float val_fl = value / 100.f;

    if(ViewGraphSet.GraphScaleX > 9.5 && val_fl < 1.5) // Увеличить степень
    {
        if(ui->PrefixScaleX_comboBox->currentIndex() > 0)
            ui->PrefixScaleX_comboBox->setCurrentIndex(ui->PrefixScaleX_comboBox->currentIndex()-1);
    }
    else
    if(ViewGraphSet.GraphScaleX < 1.5 && val_fl > 9.5) // Уменьшить степень
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
    float val_fl = value / 100.f;

    if(ViewGraphSet.ChannelShiftY > 9.5 && val_fl < 1.5) // Увеличить степень
    {
        ViewGraphSet.DialTurnoversY ++;
    }
    else
        if(ViewGraphSet.ChannelShiftY < 1.5 && val_fl > 9.5) // Уменьшить степень
        {
            ViewGraphSet.DialTurnoversY --;
        }

    qDebug() << ViewGraphSet.DialTurnoversY << val_fl;

    ViewGraphSet.ChannelShiftY = val_fl;

    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
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
    emit StartPause_Signal();
}

/* SLIDERS_PRESSED() */

void ControlPanel::on_ChannalPosition_dial_sliderPressed()
{
    if(TimerKnobClick.elapsed() < 300)
    {
        ui->ChannalPosition_dial->setValue(0);
        ViewGraphSet.DialTurnoversY = 0;
        ViewGraphSet.ShiftMid_y = 0;
        ViewGraphSet.ChannelShiftY = 0;
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
        ui->GraphPosition_dial->setValue(0);
        ViewGraphSet.DialTurnoversX = 0;
        ViewGraphSet.ShiftMid_x = 0;
        ViewGraphSet.GraphShiftX = 0;
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
        ui->GraphScale_dial->setValue(100);
        ui->GraphScale_dial->setEnabled(false);
        KnobBlockFlag = true;
        UItimer->start(100);
    }
    TimerKnobClick.restart();
}




void ControlPanel::on_pushButtonTest_clicked()
{
    emit TestPushButton_Signal();
}


void ControlPanel::on_Allways_rb_clicked()
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    emit ChangeParseMode_Signal(0, chan_num);
}


void ControlPanel::on_Single_rb_clicked()
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    emit ChangeParseMode_Signal(1, chan_num);
}


void ControlPanel::on_Trigger_rb_clicked()
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    emit ChangeParseMode_Signal(2, chan_num);
}

#if 0
void ControlPanel::on_TriggerPosition_dial_valueChanged(int value)
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    float fl_val = value / 100.f;
    emit TriggerChanged_Signal(chan_num, fl_val);
}
#endif

void ControlPanel::on_SetHalf_Button_clicked()
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    emit ClickHalfTrig_Signal(chan_num);
}

void ControlPanel::SetTrigValue(double val)
{
    //ui->TriggerPosition_Slider->setValue(100 * val);
    ui->TriggerPosition_Box->setValue(val);
    emit ui->TriggerPosition_Box->editingFinished();
}

void ControlPanel::on_AutoSize_Button_clicked()
{
    int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
    qDebug() << "as)" << chan_num;
    emit AutoSize_Signal(chan_num);
}

void getMantissaAndExponent(float num, float& mantissa, int& exponent)
{   // Естественно эту функцию можно оптимизировать, но она используется только при нажатии кнопки 1 раз, так что можно оставить
    QString strNum = QString::number(num, 'e', 3); // Преобразование числа в QString в экспоненциальной форме с точностью до 3 знаков после запятой
    int indexOfE = strNum.indexOf('e', 0, Qt::CaseSensitive); // Находим индекс символа 'e'

    // Получаем мантиссу и порядок из строки
    mantissa = strNum.left(indexOfE).toFloat();
    exponent = strNum.mid(indexOfE + 1).toInt();
}


void ControlPanel::SetDialPositionScale(float x, float y, float dx, float dy)
{
    qDebug() << x << y << dx << dy;

    ViewGraphSet.DialTurnoversX = x/10;
    ViewGraphSet.DialTurnoversY = y/10;
    x = x - ViewGraphSet.DialTurnoversX * 10;
    y = y - ViewGraphSet.DialTurnoversY * 10;

    // qDebug() << ViewGraphSet.DialTurnoversX << ViewGraphSet.DialTurnoversY;

    if(x < 0)
    {
        ViewGraphSet.DialTurnoversX = -1;
    }

    if(y < 0)
    {
        ViewGraphSet.DialTurnoversY = -1;
    }

    ui->GraphPosition_dial->setValue(x * 100);

    ui->ChannalPosition_dial->setValue(y * 100);

    float mantissa;
    int degree;

    getMantissaAndExponent(dx, mantissa, degree); // 72.4545 : 7.245 1

    // qDebug() << "ms" << mantissa << degree;

    ViewGraphSet.ScalePrefixX = pow(10, degree);
    ui->PrefixScaleX_comboBox->setCurrentIndex(3 - degree);

    ui->GraphScale_dial->setValue(mantissa * 100);

    ui->ChannalScale_dial->setValue(dy * 100);

    // ViewGraphSet.GraphShiftX = x; //  Не обязательно, т.к. создаться событие valueChanged
    // ViewGraphSet.ChannelShiftY = y;
}


void ControlPanel::on_MaxPoint_Slider_valueChanged(int value)
{
    ui->MaxPoint_Box->setValue(value);
    emit ChangeMaxPoint(value);
}


void ControlPanel::on_ChannelSelection_comboBox_currentIndexChanged(int index)
{
    int ShiftY = ui->ChannalPosition_dial->value(),
        ScaleY = ui->ChannalScale_dial->value(),
        DialTurn = ViewGraphSet.DialTurnoversY;

    emit LoadDialPosition(index, LastIndex, ShiftY, DialTurn, ScaleY, NowChannelColorButton);

    LastIndex = index;
    ViewGraphSet.DialTurnoversY = DialTurn;
    ui->ChannalPosition_dial->setValue(ShiftY);
    ui->ChannalScale_dial->setValue(ScaleY);

    SetChannelColor(NowChannelColorButton);
}


void ControlPanel::on_UpdateFill_CheckBox_stateChanged(int arg1)
{
    if(arg1 == 2)
    {
        emit ChangeUpdateSet(1, ui->UpdateFill_spinBox->value());
    }
    else
    {
        emit ChangeUpdateSet(0, ui->UpdateFill_spinBox->value());
    }
}


void ControlPanel::on_UpdateFill_spinBox_valueChanged(int arg1)
{
    if(ui->UpdateFill_CheckBox->checkState() == 0)
    {
        emit ChangeUpdateSet(0, ui->UpdateFill_spinBox->value());
    }
}


void ControlPanel::on_TrigChannel_comboBox_currentIndexChanged(int index)
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    emit ChangeParseMode_Signal(-1, chan_num);
}


void ControlPanel::on_TriggerPosition_Slider_valueChanged(int value)
{
    if(!TriggerTextFlag)
    {
        const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
        TriggerValue = value / 100.;
        ui->TriggerPosition_Box->setValue(TriggerValue);
        emit TriggerChanged_Signal(chan_num, TriggerValue);
    }
    TriggerTextFlag = false;
}

void ControlPanel::on_TriggerPosition_Box_editingFinished()
{
    const int chan_num = ui->TrigChannel_comboBox->currentData().value<int>();
    TriggerTextFlag = true;
    TriggerValue = ui->TriggerPosition_Box->value();

    emit TriggerChanged_Signal(chan_num, TriggerValue);

    ui->TriggerPosition_Slider->setValue(TriggerValue * 100);
}

void ControlPanel::SetChannelColor(QColor &clr)
{
    QString scl = "rgb(" + QString::number(clr.red()) + ", " +
                  QString::number(clr.green()) + ", " +
                  QString::number(clr.blue()) + ")" ;

    ui->ChannelColor_Button->setStyleSheet(
        "QPushButton{border-radius: 4px;background-color: " + scl + ";}"
        );
}

void ControlPanel::on_ChannelColor_Button_clicked()
{
    NowChannelColorButton = QColorDialog::getColor(NowChannelColorButton, this, "Choose Color");
    if(NowChannelColorButton.isValid())
    {
        SetChannelColor(NowChannelColorButton);

        int chan_num = ui->ChannelSelection_comboBox->currentData().value<int>();
        emit ChangeChannelColor(chan_num, NowChannelColorButton);
    }
}

