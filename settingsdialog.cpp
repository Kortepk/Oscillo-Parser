#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "QSerialPort"
#include <QSerialPortInfo>
#include <QFile>
#include <QDebug>
#include <QErrorMessage>
#include <QSettings>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QValidator *validator = new QIntValidator(1, 99999999, this);
    ui->baudRateBox->setValidator(validator);

    fillPortsParameters();
    fillPortsInfo();
    LoadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

// CloseEvent не нужен

void SettingsDialog::LoadSettings()
{
    QFile file("comport.ini");

    if (!file.exists()) { // Файл не существует
        file.open(QIODevice::ReadWrite);

        if (file.isOpen()) {
            QTextStream stream(&file);
            stream << "[port_parametrs]\n"
                      "PortName=COM1\n"
                      "Baudrate=115200\n"
                      "DataBits=8\n"
                      "Parity=None\n"
                      "StopBits=1\n"
                      "FlowControl=None";

            file.close();
        } else {
            (new QErrorMessage(this))->showMessage("Failed to create file.");
            return;
        }
    }

    QSettings iniSettings("comport.ini", QSettings::IniFormat);

    iniSettings.beginGroup("port_parametrs");
    m_currentSettings.name = iniSettings.value("PortName", "COM1").toString();
    m_currentSettings.stringBaudRate = iniSettings.value("Baudrate", "115200").toString();
    m_currentSettings.stringDataBits = iniSettings.value("DataBits", "8").toString();
    m_currentSettings.stringParity = iniSettings.value("Parity", "None").toString();
    m_currentSettings.stringStopBits = iniSettings.value("StopBits", "1").toString();
    m_currentSettings.stringFlowControl = iniSettings.value("FlowControl", "None").toString();
    iniSettings.endGroup();

    // Set parametrs in box
    ui->baudRateBox->setText(m_currentSettings.stringBaudRate);
    ui->dataBitsBox->setCurrentText(m_currentSettings.stringDataBits);
    ui->parityBox->setCurrentText(m_currentSettings.stringParity);
    ui->stopBitsBox->setCurrentText(m_currentSettings.stringStopBits);
    ui->flowControlBox->setCurrentText(m_currentSettings.stringFlowControl);
    ui->serialPortInfoListBox->setCurrentText(m_currentSettings.name);

    m_currentSettings.baudRate = m_currentSettings.stringBaudRate.toInt();
    const auto dataBitsData = ui->dataBitsBox->currentData();
    m_currentSettings.dataBits = dataBitsData.value<QSerialPort::DataBits>();
    const auto parityData = ui->parityBox->currentData();
    m_currentSettings.parity = parityData.value<QSerialPort::Parity>();
    const auto stopBitsData = ui->stopBitsBox->currentData();
    m_currentSettings.stopBits = stopBitsData.value<QSerialPort::StopBits>();
    const auto flowControlData = ui->flowControlBox->currentData();
    m_currentSettings.flowControl = flowControlData.value<QSerialPort::FlowControl>();
}

SettingsDialog::Settings SettingsDialog::settings()
{
    return m_currentSettings;
}

void SettingsDialog::fillPortsParameters()
{
    ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

void SettingsDialog::fillPortsInfo()
{
    ui->serialPortInfoListBox->clear();
    const QString blankString = tr(::blankString);
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        const QString description = info.description();
        const QString manufacturer = info.manufacturer();
        const QString serialNumber = info.serialNumber();
        const auto vendorId = info.vendorIdentifier();
        const auto productId = info.productIdentifier();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (vendorId ? QString::number(vendorId, 16) : blankString)
             << (productId ? QString::number(productId, 16) : blankString);

        ui->serialPortInfoListBox->addItem(list.constFirst(), list);
    }
}

void SettingsDialog::updateSettings()
{
    m_currentSettings.name = ui->serialPortInfoListBox->currentText();

    m_currentSettings.baudRate = ui->baudRateBox->text().toInt();

    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);

    const auto dataBitsData = ui->dataBitsBox->currentData();
    m_currentSettings.dataBits = dataBitsData.value<QSerialPort::DataBits>();
    m_currentSettings.stringDataBits = ui->dataBitsBox->currentText();

    const auto parityData = ui->parityBox->currentData();
    m_currentSettings.parity = parityData.value<QSerialPort::Parity>();
    m_currentSettings.stringParity = ui->parityBox->currentText();

    const auto stopBitsData = ui->stopBitsBox->currentData();
    m_currentSettings.stopBits = stopBitsData.value<QSerialPort::StopBits>();
    m_currentSettings.stringStopBits = ui->stopBitsBox->currentText();

    const auto flowControlData = ui->flowControlBox->currentData();
    m_currentSettings.flowControl = flowControlData.value<QSerialPort::FlowControl>();
    m_currentSettings.stringFlowControl = ui->flowControlBox->currentText();


    QFile file("comport.ini");
    file.open(QIODevice::WriteOnly);
    if (!file.isOpen()) {
        (new QErrorMessage(this))->showMessage("Failed to create file and write init.");
        return;
    }

    QTextStream stream(&file);
    stream << "[port_parametrs]\n"
              "PortName=" +  m_currentSettings.name + "\n"
              "Baudrate=" +  m_currentSettings.stringBaudRate + "\n"
              "DataBits=" +  m_currentSettings.stringDataBits + "\n"
              "Parity=" +  m_currentSettings.stringParity + "\n"
              "StopBits=" +  m_currentSettings.stringStopBits + "\n"
              "FlowControl=" +  m_currentSettings.stringFlowControl + "";

    file.close();
}

void SettingsDialog::on_Apply_pushButton_clicked()
{
    updateSettings();
    //TODO Save port settings from file
    hide();
}


void SettingsDialog::on_pushButton_clicked()
{
    fillPortsInfo();
}

