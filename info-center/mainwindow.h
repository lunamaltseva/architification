#pragma once
#include <QMainWindow>

class QStackedWidget;
class QButtonGroup;
class QLabel;
class QPushButton;
class QProgressBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSubscribeClicked();
    void onPayNowClicked();
    void onPaymentBack();

private:
    QWidget *createAboutPage();
    QWidget *createPaymentPage();

    QString readSysFile(const QString &path);
    QString getCpuInfo();
    QString getMemInfo();
    QString getKernelVersion();
    QString getGraphicsPlatform();
    QString getKdePlasmaVersion();
    QString getGraphicsInfo();

    QStackedWidget *m_stack;
    QButtonGroup   *m_tierGroup;
    QLabel         *m_payTotalLabel;
    QLabel         *m_paymentError;
    QLabel         *m_paymentPunchline;
    QPushButton    *m_payNowBtn;
    QProgressBar   *m_payProgress;
};
