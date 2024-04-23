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

QSpacerItem *refOscilloSpacer;

QSerialPort* MainPort = nullptr;

QByteArray RxBuffer;

QTimer *UpdateGraphTimer;
QElapsedTimer Maintimer;
QPointF TempPoint;

auto start_time = std::chrono::high_resolution_clock::now();
QElapsedTimer MainTimer;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
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

    ConcreteChannels[0].initSine();

    QVBoxLayout *groupBoxLayout = new QVBoxLayout();
    groupBoxLayout->addWidget(ConcreteChannels[0].ChartView_pointer); // Создаём текст внутри GroupBox

    delete ui->Channel1_groupBox->layout(); // Удаляем layout от дизайнера
    ui->Channel1_groupBox->setLayout(groupBoxLayout);

    refOscilloSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);

    ConcreteChannels[0].QGroupBox_pointer = ui->Channel1_groupBox; // Main oscillo GroupBox


    //ui->OscilloPanel_Layout->setContentsMargins(0, 0, 0, 0); // Установка нулевых отступов
    // Добавление виджета в макет окна
    ui->OscilloPanel_Layout->addWidget(ControlPnl);


    //dialog.show();
    // Отображение нового окна

    UpdateGraphTimer = new QTimer(this);
    connect(UpdateGraphTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::UpdateGraph));
    UpdateGraphTimer->start(1000/75); // /75
}

void MainWindow::UpdateGraph(void)
{

    if(MainPort->isOpen()) // 2 for trigger
    {
        for(int i = 0; i <= Channel_Size; i++)
        {
            if(ConcreteChannels[i].fillingIndex >= 2)
            ConcreteChannels[i].ReplaceDots();
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
    connect(ControlPnl, &ControlPanel::ChangeMaxPoint, this, &MainWindow::ChangeMaxPointFunc);
    connect(ControlPnl, &ControlPanel::ChangeUpdateSet, this, &MainWindow::ChangePointUpdate);
    connect(ControlPnl, &ControlPanel::LoadDialPosition, this, &MainWindow::LoadSetDial);
    connect(ControlPnl, &ControlPanel::ChangeChannelColor, this, &MainWindow::SetChannelColor);

    connect(ControlPnlDialog, &QDialog::finished, this, &MainWindow::CloseFlowPanel);

    for(int i=0; i<10; i++) // Соединяем каналы у всех сигналов
    {
        connect(&ConcreteChannels[i], &oscillo_channel::MasterHandle_Signal, this, &MainWindow::SwitchRecieveMaster);
        connect(&ConcreteChannels[i], &oscillo_channel::OverloadPoints, this, &MainWindow::OverloadPointsHandler);
    }
}

void MainWindow::SetChannelColor(int channel, QColor clr)
{
    ConcreteChannels[channel].SetChannelColor(clr);
}

void MainWindow::LoadSetDial(int chn, int lastchn, int& DialShiftY, int& DialTurn, int& DialScaleY, QColor& clr)
{
    ConcreteChannels[lastchn].ChannelShiftY = DialShiftY; // Запоминаем старые значения для старого канала
    ConcreteChannels[lastchn].ChannelScaleY = DialScaleY;
    ConcreteChannels[lastchn].ChannelTurnY  = DialTurn;
    // Цвет не записываю

    DialShiftY = ConcreteChannels[chn].ChannelShiftY; // Ставим новыее значения
    DialScaleY = ConcreteChannels[chn].ChannelScaleY;
    DialTurn   = ConcreteChannels[chn].ChannelTurnY;
    clr        = ConcreteChannels[chn].ChanColor;
}


void MainWindow::ChangePointUpdate(int state, int value)
{
    UpdWhenFill = state;

    if(UpdWhenFill)
    {
        UpdateGraphTimer->stop();
    }
    else
    {
        UpdateGraphTimer->start(1000/value);
    }
}

void MainWindow::OverloadPointsHandler()
{
    UpdateGraph();

    for(int i=0; i <= Channel_Size; i++)
    {
        ConcreteChannels[i].SaveLastValue();

        ConcreteChannels[i].LastTime = 0;       // При переполнение одного из канала, мы синхронизируем другие
        ConcreteChannels[i].fillingIndex = 0;

        switch(TriggerMode)
        {
        case 0:
            {
                ConcreteChannels[i].ModeMaster = false;
                ConcreteChannels[i].AddPointFlag = true;
                break;
            }
        case 1:
        case 2:
            {
                if(ConcreteChannels[i].ModeMaster) // Если обнуляемый канал - мастер, то он продолжаем приём
                    ConcreteChannels[i].AddPointFlag = true;
                else
                    ConcreteChannels[i].AddPointFlag = false;
                break;
            }
        }
    }
    if((TriggerMode == 1) && PortReadFlag)
        ControlPnl->on_StartPause_Button_clicked(); // Ставим на паузу
}


void MainWindow::SwitchRecieveMaster(bool status)
{
    for(int i=0; i < Channel_Size; i++)
        ConcreteChannels[i].AddPointFlag = status;
}


void MainWindow::ChangeMaxPointFunc(int pointSize)
{
    for(int i=0; i < Channel_Size; i++)
    {
        ConcreteChannels[i].MaxPoint = pointSize;
    }
}

void MainWindow::AutoSizeClick(int channel)
{
    if(channel > Channel_Size) // Валидация на всякий случий
        return;


    float LastMinPoint = ConcreteChannels[channel].LastMinPoint;
    float LastMaxPoint = ConcreteChannels[channel].LastMaxPoint;
    const float LastMaxTime  = ConcreteChannels[channel].LastMaxTime;

    if(LastMinPoint != LastMaxPoint) // Есть хоть какие-то значения
    {
        float HalfVal = (LastMaxPoint + LastMinPoint)/2;
        ControlPnl->SetDialPositionScale(5, HalfVal, LastMaxTime, (LastMaxPoint - LastMinPoint) * 1.05);
    }
}

void MainWindow::CalcHalfTrigger(int channel)
{
    if(channel > Channel_Size) // Валидация на всякий случий
        return;

    double HalfVal = (ConcreteChannels[channel].LastMaxPoint + ConcreteChannels[channel].LastMinPoint)/2;
    ControlPnl->SetTrigValue(HalfVal);

    MainWindow::ChangeParsingMode(TriggerMode, channel + 1);
}

void MainWindow::TrigerValueChanged(int channel, double val)
{
    TriggerValue = val;
    qDebug() << "TrgVal" << TriggerValue;
    MainWindow::ChangeParsingMode(TriggerMode, channel);
}

void MainWindow::ChangeParsingMode(int mode, int channel)
{
    if(mode >= 0)
        TriggerMode = mode;

    if(channel > Channel_Size) // Валидация на всякий случий
        return;

    //qDebug() << TriggerMode <<  TriggerValue << channel;

    ConcreteChannels[channel].TriggerValue = TriggerValue;
    if(mode > 0) // По триггеру
        ConcreteChannels[channel].ModeMaster = true;
    else
        ConcreteChannels[channel].ModeMaster = false;

    OverloadPointsHandler(); // Обнуляем состояния

}

void MainWindow::TestFunction()
{
    //for(int i=0; i < ConcreteChannels[0].ListPoint.size(); i++)
    //    qDebug() << ConcreteChannels[0].ListPoint.at(i);
}

void MainWindow::StartPauseReadData()
{
    PortReadFlag = !PortReadFlag; // true - кнопка горит красным, false - зелёным
}


void MainWindow::ChangeGraph(int channel)//float shift_x, float shift_y, float scale_x, float scale_y)
{
    float WindSizeX = (ControlPnl->ViewGraphSet.GraphScaleX * ControlPnl->ViewGraphSet.ScalePrefixX);
    if(channel < 0) // Если мы производим настройку по времени
    {   // (0.0; 1.0)  * (max_x - min_x)
        ControlPnl->ViewGraphSet.ShiftMid_x = ControlPnl->ViewGraphSet.GraphShiftX * WindSizeX / 10.f + WindSizeX * ControlPnl->ViewGraphSet.DialTurnoversX; // 1 оборот - 1 крутка экрана
    }

    float min_x = ControlPnl->ViewGraphSet.ShiftMid_x - WindSizeX/2,
          min_y = 0,
          max_x = ControlPnl->ViewGraphSet.ShiftMid_x + WindSizeX/2,
          max_y = 0;


    if(channel >= 0) // Если дан конкретный канал
    {
        float shift_y = ControlPnl->ViewGraphSet.ChannelShiftY + 10 * ControlPnl->ViewGraphSet.DialTurnoversY;
        min_y = shift_y - ControlPnl->ViewGraphSet.ChannelScaleY/2;
        max_y = shift_y + ControlPnl->ViewGraphSet.ChannelScaleY/2;

        ConcreteChannels[channel].SetGraphAxis(&min_x, &min_y, &max_x, &max_y);
    }
    else
        for(int i = 0; i <= Channel_Size; i ++)
        {
            QtCharts::QAbstractAxis *oldAxisY = ConcreteChannels[i].ChartView_pointer->chart()->axes(Qt::Vertical).at(0);

            QtCharts::QValueAxis  * axisY = static_cast<QtCharts::QValueAxis * >(oldAxisY);
            min_y = axisY->min();
            max_y = axisY->max();

           ConcreteChannels[i].SetGraphAxis(&min_x, &min_y, &max_x, &max_y);
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
    if(!PortReadFlag)
    {
        MainPort->clear();
        MainTimer.restart();
        return;
    }

    if(MainTimer.elapsed() < 10)
        return;

    RxBuffer += MainPort->readAll();

    while((RxBuffer[0] != ';') && (RxBuffer.length() > 0)) // Условие синхронизации, относительно последнего числа и символа ;
    {
        RxBuffer.remove(0, 1);
    }

    int indexEOF = 0, indexSOF = 0;
    QString str, strtm;
    float avergePacketTime, deltaTime;

    const static int CountPacketSep = RxBuffer.count(';') - 1; // Считаем сколько всего полных пакетов - нач символ
    const static int CountTimeSep = RxBuffer.count(':');       // Считаем сколько отметок о времени есть

    const int NotConTime = CountPacketSep - CountTimeSep; // Не содержащие отметки о времени

    if(NotConTime > 0)
        avergePacketTime = (MainTimer.nsecsElapsed() * 0.000001) / NotConTime; // Дельта времение от предыдущего приёма / на все пакеты


    while(indexSOF < RxBuffer.length())
    {
        indexEOF = RxBuffer.indexOf(';', 1); // Пропускаем [0] т.к. символ ';'
        if (indexEOF != -1) {

            if(indexEOF > 2) // [1] - номер канала; [2] - ')'
            {
                int NumberChannel = RxBuffer[1] - '0';
                int TimeSeparator = RxBuffer.indexOf(':', 0);

                if((TimeSeparator == -1) || (TimeSeparator > indexEOF)) // Уходит к другому пакету
                {
                    str = RxBuffer.mid(3, indexEOF-3);

                    if(NumberChannel <= Channel_Size)
                        ConcreteChannels[NumberChannel].LastTime += avergePacketTime; // Здесь мы устанавливаем новое значение, на котором отобразиться точка

                    deltaTime = 0; // Сдвига в этом случае не будет
                }
                else
                {
                    // TODO: Добавить степеное изменение времени
                    str = RxBuffer.mid(3, TimeSeparator - 3);
                    strtm = RxBuffer.mid(TimeSeparator+1, indexEOF - TimeSeparator-1);
                    deltaTime = strtm.toFloat();
                }

                if(NumberChannel <= Channel_Size)
                    ConcreteChannels[NumberChannel].ValueProcessing(str.toFloat(), deltaTime);
            }
        }
        else
            break;
        RxBuffer.remove(0, indexEOF);
    }

    MainTimer.restart();
}

void MainWindow::ChangeGroupSize(int val)
{
    for(int i=0; i < Channel_Size; i++)
        ConcreteChannels[i].ChangeGroupSize(val);
}

void MainWindow::CounterChannel_Changed(int arg1)
{
    if(Channel_Size < arg1) // Количество каналов увеличилось
    {
        Channel_Size = arg1;
        const int chn = Channel_Size;

        if(ConcreteChannels[chn].QGroupBox_pointer ==  nullptr)
        {
            if(!MainPort->isOpen())
                ConcreteChannels[chn].initSine();
            else
                ConcreteChannels[chn].ConnectSeries();

            ConcreteChannels[chn].CreateGroupBox("Channel " + QString::number(arg1), ControlPnl->Get_GroupSizeValue());

            QVBoxLayout *groupBoxLayout = new QVBoxLayout();
            groupBoxLayout->addWidget(ConcreteChannels[chn].ChartView_pointer);
            ConcreteChannels[chn].QGroupBox_pointer->setLayout(groupBoxLayout);

            ui->Oscillo_Channel_Area_verticalLayout->addWidget(ConcreteChannels[chn].QGroupBox_pointer); // Добавляем GB

            ui->Oscillo_Channel_Area_verticalLayout->removeItem(refOscilloSpacer);
            ui->Oscillo_Channel_Area_verticalLayout->addItem(refOscilloSpacer);
        }
        else
        {
            ConcreteChannels[chn].QGroupBox_pointer->show();
        }
    }
    else
    {
        ConcreteChannels[Channel_Size].DisconnectSeries();
        Channel_Size = arg1;
    }
}



void MainWindow::on_Connect_action_triggered()
{
    if(!MainPort->isOpen())
    {
        SettingsDialog::Settings p = SetDial->m_currentSettings;
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

        for(int i=1;i<=Channel_Size;i++){
            ConcreteChannels[i].ClearPoints();
        }

        if(!PortReadFlag)
        {
            ControlPnl->on_StartPause_Button_clicked(); // При подключении включаем приём
        }

        MainPort->clear();
        MainTimer.restart();
        ui->Connect_action->setText("Disonnect");

        OverloadPointsHandler(); // Запускаем с необходимыми триггер-параметрами
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
    for(int i = 0; i<=10; i++)
    {
        ConcreteChannels[i].DeleteElements();
    }

    if(SetDial != nullptr)
        delete SetDial;
    if(ControlPnlDialog != nullptr)
        delete ControlPnlDialog;
}
