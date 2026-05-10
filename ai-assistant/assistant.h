#pragma once
#include <QMainWindow>

class QScrollArea;
class QVBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;
class QStackedWidget;
class QWidget;

class Assistant : public QMainWindow {
    Q_OBJECT
public:
    explicit Assistant(QWidget *parent = nullptr);

private slots:
    void onSend();
    void onThinkingTick();
    void deliverResponse();
    void clearChat();

private:
    void sendPrompt(const QString &text);
    void addMessage(const QString &text, bool isUser);
    void addThinkingIndicator();
    void removeThinkingIndicator();
    QString buildResponse(const QString &prompt);
    bool isSpotifyQuery(const QString &prompt);
    void handleSpotifyQuery(const QString &prompt);
    QWidget *buildHeroPage();

    QStackedWidget *m_pages;
    QScrollArea    *m_scrollArea;
    QWidget        *m_messagesWidget;
    QVBoxLayout    *m_messagesLayout;
    QWidget        *m_thinkingWidget;
    QLabel         *m_thinkingLabel;

    QLineEdit      *m_input;
    QPushButton    *m_sendBtn;
    QPushButton    *m_clearBtn;
    QTimer         *m_thinkingTimer;

    int     m_thinkingTick;
    QString m_pendingPrompt;
    int     m_responseIndex;
};
