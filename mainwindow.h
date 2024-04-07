#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ControlPanel.h"
#include "oscillo_channel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ChangeFlowWindowMode(bool TriggerStatus);

private slots:
    void CounterChannel_Changed(int arg1);
    void on_Connect_action_triggered();
    void on_PortSettings_action_triggered();
    void readData();

private:
    Ui::MainWindow *ui;
    void ChangeGroupSize(int val);
    void initActionsConnections(void);
    void initSine(int channel);
    void UpdateGraph(void);
    void initSettings(void);
    void CloseFlowPanel(void);
    void closeEvent(QCloseEvent *event);
    void ChangeGraph(int channel);
    void StartPauseReadData();
    void TestFunction();
    void ChangeParsingMode(int mode, int channel);
    void TrigerValueChanged(int channel, float val);
    void CalcHalfTrigger(int channel);
    void AutoSizeClick(int channel);
    void ChangeMaxPointFunc(int pointSize);
    void SwitchRecieveMaster(bool status);
    void OverloadPointsHandler();
    void ChangePointUpdate(int state, int value);

    SettingsDialog *SetDial = nullptr;
    SettingsDialog *m_settings = nullptr;
    ControlPanel *ControlPnl = nullptr;
    QDialog *ControlPnlDialog = nullptr;
    oscillo_channel ConcreteChannels[10];

    int Channel_Size = 1; // Указывает сколько сейчас каналов
    int TriggerMode = 0;  // 0 = Allways; 1 = Single; 2 = By trigger
    float TriggerValue = 0;
    bool PortReadFlag = true;
    bool UpdWhenFill = false; // updating when filling out

};
#endif // MAINWINDOW_H
