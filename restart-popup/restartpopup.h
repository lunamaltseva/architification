#pragma once
#include <QObject>

class QTimer;
class QDialog;
class QWidget;

class RestartPopup : public QObject {
    Q_OBJECT
public:
    explicit RestartPopup(QObject *parent = nullptr);
    void start();

private slots:
    void showPopup();

private:
    QDialog *buildDialog(int level, bool threatening);
    void showFakeShutdown();

    QTimer *m_timer;
    int  m_dismissed;       // total dismissals — determines level
    int  m_chainDismissals; // dismissals once chaining starts (level 4+)
    bool m_stopped;         // set true after fake shutdown; nothing fires again
};
