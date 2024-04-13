#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
    } m_currentSettings;
    void LoadSettings(void);

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings settings();

private slots:
    void on_Apply_pushButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::SettingsDialog *ui;
    void fillPortsParameters(void);
    void fillPortsInfo(void);
    void updateSettings(void);

    //Settings m_currentSettings;
};

#endif // SETTINGSDIALOG_H
