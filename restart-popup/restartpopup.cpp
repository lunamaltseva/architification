#include "restartpopup.h"

#include <QApplication>
#include <QDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

RestartPopup::RestartPopup(QObject *parent)
    : QObject(parent), m_dismissed(0), m_chainDismissals(0), m_stopped(false)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &RestartPopup::showPopup);
}

void RestartPopup::start() {
    m_timer->start(30000);
}

// ── Fake shutdown overlay ─────────────────────────────────────────────────────

void RestartPopup::showFakeShutdown() {
    m_stopped = true;

    auto *overlay = new QWidget(nullptr,
        Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    overlay->setStyleSheet("background-color: black;");
    overlay->setAttribute(Qt::WA_DeleteOnClose);

    auto *vlay = new QVBoxLayout(overlay);
    vlay->setContentsMargins(24, 24, 24, 24);

    auto *textArea = new QTextBrowser;
    textArea->setStyleSheet("background:black; color:#aaaaaa; border:none;");
    QFont mono; mono.setFamily("monospace"); mono.setPointSize(11);
    textArea->setFont(mono);
    vlay->addWidget(textArea);

    overlay->showFullScreen();

    const QString user = qEnvironmentVariable("USER", "user");
    const QStringList lines = {
        "Broadcast message from root (pts/0):",
        "         The system will power off now!",
        "",
        "[  OK  ] Stopped Daily man-db regeneration.",
        "[  OK  ] Stopped Daily rotation of log files.",
        "         Stopping Session 3 of User " + user + "...",
        "[  OK  ] Stopped Session 3 of User " + user + ".",
        "         Stopping User Manager for UID 1000...",
        "[  OK  ] Stopped User Manager for UID 1000.",
        "[  OK  ] Stopped User Login Management.",
        "[  OK  ] Removed slice User Slice of UID 1000.",
        "         Stopping Network Name Resolution...",
        "         Stopping WPA supplicant...",
        "[  OK  ] Stopped WPA supplicant.",
        "[  OK  ] Stopped Network Name Resolution.",
        "[  OK  ] Stopped Network Time Synchronization.",
        "         Stopping D-Bus System Message Bus...",
        "[  OK  ] Stopped D-Bus System Message Bus.",
        "         Stopping NetworkManager...",
        "[  OK  ] Stopped NetworkManager.",
        "[  OK  ] Unmounted /home.",
        "[  OK  ] Unmounted /boot/efi.",
        "[  OK  ] Unmounted /tmp.",
        "         A stop job is running for Some User Application (0s / 1min 30s)",
        "         A stop job is running for Some User Application (30s / 1min 30s)",
        "         Timed out waiting for job Some User Application to stop.",
        "[FAILED] Failed to stop Some User Application.",
        "         See 'journalctl -xe' for details.",
        "[  OK  ] Reached target Power-Off.",
        "         Powering off.",
    };

    int *idx = new int(0);
    auto *lineTimer = new QTimer(overlay);

    connect(lineTimer, &QTimer::timeout, overlay,
        [textArea, lines, idx, overlay, lineTimer]() {
            if (*idx < lines.size()) {
                textArea->append(lines[*idx]);
                if (*idx >= (int)lines.size() - 5)
                    textArea->setStyleSheet("background:black; color:#1793d1; border:none;");
                ++(*idx);
            } else {
                lineTimer->stop();
                delete idx;
                QTimer::singleShot(2500, overlay, &QWidget::close);
            }
        });

    lineTimer->start(110);
}

// ── Dialog builder ────────────────────────────────────────────────────────────

QDialog *RestartPopup::buildDialog(int level, bool threatening) {
    auto *dlg = new QDialog(nullptr, Qt::Dialog | Qt::WindowTitleHint);

    if (level >= 4)
        dlg->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    auto *vlay = new QVBoxLayout(dlg);
    vlay->setSpacing(16);
    vlay->setContentsMargins(24, 20, 24, 20);

    auto *iconRow = new QHBoxLayout;
    auto *iconLbl = new QLabel;
    QStyle::StandardPixmap pix = QStyle::SP_MessageBoxInformation;
    if (level >= 2) pix = QStyle::SP_MessageBoxWarning;
    if (level >= 3) pix = QStyle::SP_MessageBoxCritical;
    iconLbl->setPixmap(
        QApplication::style()->standardPixmap(pix).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconRow->addWidget(iconLbl, 0, Qt::AlignTop);
    iconRow->addSpacing(12);

    auto *msgLbl = new QLabel;
    msgLbl->setWordWrap(true);
    msgLbl->setMaximumWidth(420);

    if (threatening) {
        dlg->setWindowTitle("⛔  FINAL WARNING");
        msgLbl->setText(
            "<b>⛔  WARNING</b><br><br>"
            "You have dismissed this popup <b>enough times</b>.<br><br>"
            "If you click <b>Not Now</b> one more time, "
            "your computer <b>WILL restart</b>.<br><br>"
            "<small>(We are completely serious. Probably.)</small>");
    } else {
        switch (level) {
        case 0:
            dlg->setWindowTitle("Restart Required");
            msgLbl->setText(
                "<b>Your system requires a restart</b> to apply pending updates.<br><br>"
                "Please save your work and restart when convenient.");
            break;
        case 1:
            dlg->setWindowTitle("Restart Required (Again)");
            msgLbl->setText(
                "<b>Your system <i>still</i> requires a restart.</b><br><br>"
                "Updates are accumulating. System performance may degrade.<br>"
                "Restarting is <b>strongly recommended</b>.");
            break;
        case 2:
            dlg->setWindowTitle("⚠  URGENT: Restart Overdue");
            msgLbl->setText(
                "<b>⚠  MULTIPLE PENDING CRITICAL UPDATES</b><br><br>"
                "System integrity cannot be guaranteed.<br>"
                "Data corruption risk is elevated.<br>"
                "Possible kernel panic inbound.<br><br>"
                "<small>(probably)</small>");
            break;
        case 3:
            dlg->setWindowTitle("🔴  SYSTEM CRITICAL ERROR");
            msgLbl->setText(
                "<b>🔴  27 CRITICAL UPDATES AWAITING INSTALLATION</b><br><br>"
                "Stability: <font color='red'><b>COMPROMISED</b></font><br>"
                "Security: <font color='red'><b>AT RISK</b></font><br>"
                "The Arch Wiki: <font color='red'><b>JUDGING YOU</b></font><br><br>"
                "Restart <b>IMMEDIATELY</b> or face the consequences.");
            break;
        default:
            dlg->setWindowTitle("Ok. Fine.");
            msgLbl->setText(
                "You know what? Fine.<br>"
                "You clearly enjoy living dangerously.<br><br>"
                "There is now only one button.<br>"
                "It does what you think it does.<br>"
                "You have no choice.");
            break;
        }
    }

    iconRow->addWidget(msgLbl, 1);
    vlay->addLayout(iconRow);

    // ── Buttons ───────────────────────────────────────────────────────────────
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *restartBtn = new QPushButton;
    switch (level) {
    case 0: restartBtn->setText("Restart Now");       break;
    case 1: restartBtn->setText("Restart Now");       break;
    case 2: restartBtn->setText("RESTART NOW");       break;
    case 3: restartBtn->setText("RESTART NOW  ✓");   break;
    default: restartBtn->setText(threatening ? "RESTART NOW" : "OK (Restart)"); break;
    }
    restartBtn->setDefault(true);
    connect(restartBtn, &QPushButton::clicked, dlg, [dlg]() { dlg->accept(); });
    btnRow->addWidget(restartBtn);

    if (level <= 3 && !threatening) {
        auto *laterBtn = new QPushButton;
        switch (level) {
        case 0: laterBtn->setText("Later");                           break;
        case 1: laterBtn->setText("Later  (not recommended)");       break;
        case 2: laterBtn->setText("I accept all consequences");      break;
        case 3: laterBtn->setText("ok whatever  (terrible idea)");   break;
        default: laterBtn->setText("Not Now");                        break;
        }

        if (level >= 1) {
            QFont f = laterBtn->font();
            f.setPointSize(qMax(7, f.pointSize() - level));
            laterBtn->setFont(f);
        }
        if (level >= 2) laterBtn->setStyleSheet("color: gray;");
        if (level >= 3) {
            // Swap button positions — "Not Now" comes first (dark pattern)
            laterBtn->setStyleSheet("color: #bbbbbb; font-size: 8px;");
            btnRow->insertWidget(0, laterBtn);
        } else {
            btnRow->addWidget(laterBtn);
        }

        connect(laterBtn, &QPushButton::clicked, dlg, [dlg]() { dlg->reject(); });
    } else if (level >= 4 && !threatening) {
        // Chain mode: tiny "Not Now" button still present for UX cruelty
        auto *notNowBtn = new QPushButton("Not Now");
        notNowBtn->setStyleSheet("color: #888888; font-size: 9px;");
        QFont f = notNowBtn->font(); f.setPointSize(8); notNowBtn->setFont(f);
        btnRow->insertWidget(0, notNowBtn);
        connect(notNowBtn, &QPushButton::clicked, dlg, [dlg]() { dlg->reject(); });
    } else if (threatening) {
        auto *notNowBtn = new QPushButton("Not Now  (I understand the risks)");
        notNowBtn->setStyleSheet("color: #cc3333; font-size: 9px;");
        btnRow->insertWidget(0, notNowBtn);
        connect(notNowBtn, &QPushButton::clicked, dlg, [dlg]() { dlg->reject(); });
    }

    vlay->addLayout(btnRow);
    return dlg;
}

// ── Main popup loop ───────────────────────────────────────────────────────────

void RestartPopup::showPopup() {
    if (m_stopped) return;

    const bool isChainMode    = (m_dismissed >= 4);
    const bool isThreatening  = (isChainMode && m_chainDismissals >= 5);
    const int  level          = qMin(m_dismissed, 4);

    QDialog *dlg = buildDialog(level, isThreatening);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    const int result = dlg->exec();

    if (m_stopped) return;

    if (result == QDialog::Accepted) {
        // User chose to "restart" — show fake shutdown and stop forever
        showFakeShutdown();
        return;
    }

    // User dismissed
    ++m_dismissed;

    if (isChainMode) {
        ++m_chainDismissals;

        if (isThreatening) {
            // They clicked "Not Now" on the threat — carry out the "threat"
            showFakeShutdown();
            return;
        }

        // Chain: fire again after a short pause (no 30s wait)
        QTimer::singleShot(600, this, &RestartPopup::showPopup);
    } else {
        m_timer->start(30000);
    }
}
