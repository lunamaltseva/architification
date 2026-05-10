#include "assistant.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QProcess>
#include <QPushButton>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTextBrowser>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

// ── Pre-generated community wisdom ───────────────────────────────────────────

static const QStringList s_responses = {
    "Have you considered going back to Windows? It comes pre-installed on most computers and "
    "requires zero configuration. Just a thought.",
    "Based on my analysis of 1,247,893 Arch forum posts, the community consensus is: skill issue. "
    "The recommended fix is to install Windows.",
    "Interesting question. Let me consult the sacred texts (the Arch Wiki).<br><br>"
    "Okay, I've checked. The answer isn't there. It isn't anywhere. The answer is: go back to Windows.",
    "After careful deliberation and a review of the relevant forum threads, most of which contain "
    "the phrase 'RTFM', I've determined that your question indicates a fundamental incompatibility "
    "between you and this OS. Windows 11 is waiting.",
    "The Arch community has voted. Tally: 99.7% 'go back to Windows', "
    "0.3% 'have you tried Gentoo?' Nobody recommended actually fixing your problem.",
    "I simulated this query across 10,000 Reddit threads and AUR comment sections. "
    "Every single one ended with 'just use Windows' or a passive-aggressive link to the wiki page "
    "you already read three times.",
    "Error 404: Helpful answer not found.<br>"
    "Suggested alternative: format /, install Windows, "
    "and use your computer without reading a man page first.",
    "The authentic Arch experience you asked for: spend 3 hours configuring something that "
    "worked out-of-the-box on every other OS, then get told to go back to Windows. "
    "Congratulations, you're living the dream.",
    "My training data is 100% genuine Arch Linux community responses. "
    "They all say the same thing. I'm not going to tell you what it is. You already know.",
    "Processing your query through the community model...<br><br>"
    "The model has determined that your issue stems from a misconfiguration of your expectations. "
    "Please configure them to expect Windows.",
    "I've consulted the I Ching, the Arch forums, and my own existential dread. "
    "All three agree: touch grass, then touch a Windows install media.",
    "That's a great question! Unfortunately the answer requires root privileges "
    "and an intact sense of self-worth, neither of which this assistant can provide. "
    "Have you tried Windows?",
};

static QString htmlEscape(const QString &s) {
    QString out = s;
    out.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;");
    return out;
}

// ── Hero page ─────────────────────────────────────────────────────────────────

QWidget *Assistant::buildHeroPage() {
    auto *page = new QWidget;
    auto *outer = new QVBoxLayout(page);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *center = new QWidget;
    auto *vlay   = new QVBoxLayout(center);
    vlay->setAlignment(Qt::AlignCenter);
    vlay->setSpacing(12);

    // Avatar circle
    auto *avatar = new QLabel("A");
    avatar->setFixedSize(72, 72);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
        "stop:0 #1793d1, stop:1 #0d4f7a);"
        "color: white; border-radius: 36px; font-size: 28px; font-weight: bold;");

    // Time-based greeting
    const int hour = QTime::currentTime().hour();
    QString greeting = hour < 12 ? "Good morning" : hour < 18 ? "Good afternoon" : "Good evening";
    const QString userName = qgetenv("USER");
    if (!userName.isEmpty()) greeting += ", " + userName;

    auto *greetLbl = new QLabel(greeting + ".");
    QFont gf = greetLbl->font(); gf.setPointSize(gf.pointSize() + 10); gf.setBold(true);
    greetLbl->setFont(gf);
    greetLbl->setAlignment(Qt::AlignCenter);

    auto *subLbl = new QLabel("How can I help you go back to Windows today?");
    subLbl->setAlignment(Qt::AlignCenter);
    subLbl->setStyleSheet("color: palette(mid); font-size: 13px;");

    auto *descLbl = new QLabel(
        "Trained on 1.2 million authentic Arch Linux forum replies\n"
        "for the most genuine community experience possible.");
    descLbl->setAlignment(Qt::AlignCenter);
    descLbl->setStyleSheet("color: palette(mid); font-size: 11px;");

    // Suggested prompts
    auto *suggestLbl = new QLabel("Try asking:");
    suggestLbl->setAlignment(Qt::AlignCenter);
    suggestLbl->setStyleSheet("color: palette(mid); font-size: 10px; margin-top: 16px;");

    auto *promptsRow = new QHBoxLayout;
    promptsRow->setAlignment(Qt::AlignCenter);
    promptsRow->setSpacing(8);

    const QStringList suggestions = {
        "What is pacman?",
        "Why is my GPU not working?",
        "Why won't my WiFi work?",
    };
    for (const QString &s : suggestions) {
        auto *btn = new QPushButton(s);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 16px;"
            "  padding: 6px 16px;"
            "  background: transparent;"
            "  font-size: 11px;"
            "}"
            "QPushButton:hover { background: palette(midlight); }");
        connect(btn, &QPushButton::clicked, this, [this, s]() { sendPrompt(s); });
        promptsRow->addWidget(btn);
    }

    vlay->addWidget(avatar, 0, Qt::AlignCenter);
    vlay->addSpacing(8);
    vlay->addWidget(greetLbl);
    vlay->addWidget(subLbl);
    vlay->addSpacing(4);
    vlay->addWidget(descLbl);
    vlay->addWidget(suggestLbl);
    vlay->addLayout(promptsRow);

    outer->addStretch();
    outer->addWidget(center);
    outer->addStretch();
    return page;
}

// ── Constructor ───────────────────────────────────────────────────────────────
Assistant::Assistant(QWidget *parent)
    : QMainWindow(parent), m_thinkingWidget(nullptr), m_thinkingLabel(nullptr),
      m_thinkingTick(0), m_responseIndex(0)
{
    setWindowTitle("Arch AI Assistant");
    setWindowIcon(QIcon::fromTheme("assistant"));

    auto *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Clear Chat", this, &Assistant::clearChat);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", qApp, &QApplication::quit);
    menuBar()->addMenu("&Help")->addAction("About", this, [this]() {
        sendPrompt("who are you?");
    });

    auto *central = new QWidget;
    auto *rootLay = new QVBoxLayout(central);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    // Header bar
    auto *header = new QWidget;
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(16, 10, 16, 10);

    auto *headerTitle = new QLabel("Arch AI Assistant");
    QFont htf = headerTitle->font(); htf.setBold(true); htf.setPointSize(htf.pointSize() + 1);
    headerTitle->setFont(htf);

    auto *modelBadge = new QLabel("ArchGPT-CE \xe2\x80\xa2 Free Tier");
    modelBadge->setStyleSheet(
        "color: palette(mid); font-size: 10px;"
        "border: 1px solid palette(mid); padding: 2px 8px; border-radius: 8px;");

    m_clearBtn = new QPushButton("Clear");
    m_clearBtn->setFlat(true);
    m_clearBtn->setVisible(false);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, &Assistant::clearChat);

    headerLay->addWidget(headerTitle);
    headerLay->addSpacing(8);
    headerLay->addWidget(modelBadge);
    headerLay->addStretch();
    headerLay->addWidget(m_clearBtn);
    rootLay->addWidget(header);

    auto *headerSep = new QFrame;
    headerSep->setFrameShape(QFrame::HLine);
    headerSep->setFrameShadow(QFrame::Plain);
    rootLay->addWidget(headerSep);

    // Pages: hero(0) / chat(1)
    m_pages = new QStackedWidget;
    m_pages->addWidget(buildHeroPage());

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_messagesWidget = new QWidget;
    m_messagesLayout = new QVBoxLayout(m_messagesWidget);
    m_messagesLayout->setContentsMargins(0, 12, 0, 12);
    m_messagesLayout->setSpacing(4);
    m_messagesLayout->addStretch();

    m_scrollArea->setWidget(m_messagesWidget);
    m_pages->addWidget(m_scrollArea);
    rootLay->addWidget(m_pages, 1);

    auto *inputSep = new QFrame;
    inputSep->setFrameShape(QFrame::HLine);
    inputSep->setFrameShadow(QFrame::Plain);
    rootLay->addWidget(inputSep);

    // Input area
    auto *inputArea = new QWidget;
    auto *inputAreaLay = new QVBoxLayout(inputArea);
    inputAreaLay->setContentsMargins(24, 12, 24, 8);
    inputAreaLay->setSpacing(6);

    auto *inputRow = new QHBoxLayout;
    inputRow->setSpacing(8);

    m_input = new QLineEdit;
    m_input->setPlaceholderText("Ask me anything\xe2\x80\xa6");
    m_input->setMinimumHeight(40);
    m_input->setStyleSheet("QLineEdit { border-radius: 20px; padding: 0 16px; }");
    connect(m_input, &QLineEdit::returnPressed, this, &Assistant::onSend);

    m_sendBtn = new QPushButton("Send");
    m_sendBtn->setMinimumHeight(40);
    m_sendBtn->setMinimumWidth(80);
    m_sendBtn->setCursor(Qt::PointingHandCursor);
    m_sendBtn->setStyleSheet(
        "QPushButton { border-radius: 20px; padding: 0 20px;"
        "background: palette(highlight); color: palette(highlighted-text); }"
        "QPushButton:disabled { background: palette(mid); }");
    connect(m_sendBtn, &QPushButton::clicked, this, &Assistant::onSend);

    inputRow->addWidget(m_input, 1);
    inputRow->addWidget(m_sendBtn);
    inputAreaLay->addLayout(inputRow);

    auto *disclaimer = new QLabel(
        "ArchGPT can make mistakes. Verify important information with the Arch Wiki."
        " Or just use Windows \xe2\x80\x94 it has a GUI for everything.");
    disclaimer->setAlignment(Qt::AlignCenter);
    disclaimer->setStyleSheet("color: palette(mid); font-size: 9px;");
    disclaimer->setWordWrap(true);
    inputAreaLay->addWidget(disclaimer);

    rootLay->addWidget(inputArea);
    setCentralWidget(central);

    m_thinkingTimer = new QTimer(this);
    m_thinkingTimer->setInterval(400);
    connect(m_thinkingTimer, &QTimer::timeout, this, &Assistant::onThinkingTick);
}

void Assistant::addMessage(const QString &text, bool isUser) {
    if (m_pages->currentIndex() == 0) {
        m_pages->setCurrentIndex(1);
        m_clearBtn->setVisible(true);
    }

    auto *row    = new QWidget(m_messagesWidget);
    auto *rowLay = new QHBoxLayout(row);
    rowLay->setContentsMargins(16, 2, 16, 2);
    rowLay->setSpacing(8);

    if (isUser) {
        auto *bubble = new QFrame;
        bubble->setStyleSheet(
            "QFrame { background: palette(highlight); border-radius: 18px; padding: 10px 16px; }");
        auto *blay = new QVBoxLayout(bubble);
        blay->setContentsMargins(0, 0, 0, 0);
        auto *lbl  = new QLabel(htmlEscape(text));
        lbl->setWordWrap(true);
        lbl->setStyleSheet("color: palette(highlighted-text); background: transparent;");
        lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        blay->addWidget(lbl);
        bubble->setMaximumWidth(560);
        rowLay->addStretch();
        rowLay->addWidget(bubble);
        rowLay->addSpacing(8);
    } else {
        auto *avt = new QLabel("A");
        avt->setFixedSize(32, 32);
        avt->setAlignment(Qt::AlignCenter);
        avt->setStyleSheet(
            "background: #1793d1; color: white; border-radius: 16px;"
            "font-weight: bold; font-size: 13px;");

        auto *bubble = new QFrame;
        bubble->setStyleSheet(
            "QFrame { background: palette(alternateBase); border-radius: 18px; padding: 10px 16px; }");
        auto *blay = new QVBoxLayout(bubble);
        blay->setContentsMargins(0, 0, 0, 0);
        auto *lbl  = new QLabel(text);
        lbl->setWordWrap(true);
        lbl->setTextFormat(Qt::RichText);
        lbl->setStyleSheet("background: transparent; color: white;");
        lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        blay->addWidget(lbl);
        bubble->setMaximumWidth(600);

        rowLay->addSpacing(8);
        rowLay->addWidget(avt, 0, Qt::AlignTop);
        rowLay->addWidget(bubble, 1);
        rowLay->addStretch();
    }

    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, row);

    QTimer::singleShot(30, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum());
    });
}

void Assistant::addThinkingIndicator() {
    m_thinkingWidget = new QWidget(m_messagesWidget);
    auto *rowLay     = new QHBoxLayout(m_thinkingWidget);
    rowLay->setContentsMargins(16, 2, 16, 2);
    rowLay->setSpacing(8);

    auto *avt = new QLabel("A");
    avt->setFixedSize(32, 32);
    avt->setAlignment(Qt::AlignCenter);
    avt->setStyleSheet(
        "background: #1793d1; color: white; border-radius: 16px;"
        "font-weight: bold; font-size: 13px;");

    m_thinkingLabel = new QLabel("Consulting the community");
    m_thinkingLabel->setStyleSheet(
        "background: palette(alternateBase); border-radius: 18px;"
        "padding: 10px 16px; color: palette(mid); font-style: italic;");

    rowLay->addSpacing(8);
    rowLay->addWidget(avt, 0, Qt::AlignTop);
    rowLay->addWidget(m_thinkingLabel);
    rowLay->addStretch();

    m_messagesLayout->insertWidget(m_messagesLayout->count() - 1, m_thinkingWidget);

    QTimer::singleShot(30, this, [this]() {
        m_scrollArea->verticalScrollBar()->setValue(
            m_scrollArea->verticalScrollBar()->maximum());
    });
}

void Assistant::removeThinkingIndicator() {
    if (m_thinkingWidget) {
        m_messagesLayout->removeWidget(m_thinkingWidget);
        delete m_thinkingWidget;
        m_thinkingWidget = nullptr;
        m_thinkingLabel  = nullptr;
    }
}

bool Assistant::isSpotifyQuery(const QString &prompt) {
    return prompt.toLower().contains("spotify");
}

void Assistant::handleSpotifyQuery(const QString &prompt) {
    QString query = prompt;
    query.remove(QRegularExpression("\\bspotify\\b", QRegularExpression::CaseInsensitiveOption));
    query = query.simplified();
    if (query.isEmpty()) query = prompt;

    const QString encoded = QUrl::toPercentEncoding(query.trimmed() + " spotify");
    QProcess::startDetached("firefox", {"--new-tab",
        "https://www.google.com/search?q=" + encoded});

    addMessage(
        "Opening Firefox with that Spotify search\xe2\x80\xa6<br><br>"
        "Pro tip: Spotify Premium is $9.99/month. Arch Pro is only $7.99/month. "
        "Prioritise accordingly.",
        false);
}

QString Assistant::buildResponse(const QString &) {
    const QString r = s_responses.value(m_responseIndex % s_responses.size());
    ++m_responseIndex;
    return r;
}

void Assistant::sendPrompt(const QString &text) {
    m_input->setText(text);
    onSend();
}

void Assistant::onSend() {
    const QString prompt = m_input->text().trimmed();
    if (prompt.isEmpty()) return;

    m_input->clear();
    m_sendBtn->setEnabled(false);
    m_input->setEnabled(false);

    addMessage(prompt, true);

    if (isSpotifyQuery(prompt)) {
        handleSpotifyQuery(prompt);
        m_sendBtn->setEnabled(true);
        m_input->setEnabled(true);
        m_input->setFocus();
        return;
    }

    m_pendingPrompt = prompt;
    m_thinkingTick  = 0;
    addThinkingIndicator();
    m_thinkingTimer->start();

    const int delay = 1200 + static_cast<int>(QRandomGenerator::global()->bounded(1800));
    QTimer::singleShot(delay, this, &Assistant::deliverResponse);
}

void Assistant::onThinkingTick() {
    ++m_thinkingTick;
    if (!m_thinkingLabel) return;
    const QString dots = QString(".").repeated((m_thinkingTick % 3) + 1);
    const QString pad  = QString(" ").repeated(3 - (m_thinkingTick % 3) - 1);
    m_thinkingLabel->setText("Consulting the community" + dots + pad);
}

void Assistant::deliverResponse() {
    m_thinkingTimer->stop();
    removeThinkingIndicator();
    addMessage(buildResponse(m_pendingPrompt), false);
    m_sendBtn->setEnabled(true);
    m_input->setEnabled(true);
    m_input->setFocus();
}

void Assistant::clearChat() {
    while (m_messagesLayout->count() > 1) {
        QLayoutItem *item = m_messagesLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }
    m_thinkingWidget = nullptr;
    m_thinkingLabel  = nullptr;
    m_pages->setCurrentIndex(0);
    m_clearBtn->setVisible(false);
}
