#include "mainwindow.h"

#include <QApplication>
#include <QButtonGroup>
#include <QDesktopServices>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextStream>
#include <QTimer>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QRegularExpression>

// ── Sidebar helpers ──────────────────────────────────────────────────────────

static void addGroup(QListWidget *list, const QString &title) {
    auto *item = new QListWidgetItem(title, list);
    item->setFlags(Qt::NoItemFlags);
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);
    item->setSizeHint({-1, 32});
}

static void addItem(QListWidget *list, const QString &icon, const QString &text) {
    auto *item = new QListWidgetItem(QIcon::fromTheme(icon), text, list);
    item->setSizeHint({-1, 42});
}

// ── Subscription card helper ─────────────────────────────────────────────────

static QFrame *makeTierCard(const QString &name, const QString &price,
                             const QStringList &features, const QString &badge,
                             QRadioButton *radio)
{
    auto *card = new QFrame;
    card->setFrameShape(QFrame::Box);
    card->setLineWidth(1);
    card->setMinimumWidth(175);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *vlay = new QVBoxLayout(card);
    vlay->setContentsMargins(12, 10, 12, 10);
    vlay->setSpacing(5);

    if (!badge.isEmpty()) {
        auto *bl = new QLabel(badge);
        bl->setAlignment(Qt::AlignCenter);
        bl->setStyleSheet(
            "background:#f39c12;color:#000;font-size:10px;"
            "font-weight:bold;padding:2px 8px;border-radius:10px;");
        vlay->addWidget(bl);
    } else {
        vlay->addSpacing(22);
    }

    auto *nameL = new QLabel(name);
    nameL->setAlignment(Qt::AlignCenter);
    QFont nf = nameL->font(); nf.setBold(true); nf.setPointSize(nf.pointSize() + 1);
    nameL->setFont(nf);
    vlay->addWidget(nameL);

    auto *priceL = new QLabel(price);
    priceL->setAlignment(Qt::AlignCenter);
    QFont pf = priceL->font(); pf.setPointSize(pf.pointSize() + 5); pf.setBold(true);
    priceL->setFont(pf);
    vlay->addWidget(priceL);

    auto *perL = new QLabel("per month");
    perL->setAlignment(Qt::AlignCenter);
    perL->setStyleSheet("color:gray;font-size:10px;");
    vlay->addWidget(perL);

    vlay->addSpacing(6);

    for (const QString &feat : features) {
        auto *fl = new QLabel(feat);
        fl->setStyleSheet(feat.startsWith("✗") ? "color:gray;" : "");
        fl->setWordWrap(true);
        vlay->addWidget(fl);
    }

    vlay->addStretch();
    radio->setParent(card);
    vlay->addWidget(radio, 0, Qt::AlignCenter);
    return card;
}

// ── MainWindow ───────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("About this System — Info Center");
    setWindowIcon(QIcon::fromTheme("hwinfo"));

    auto *tb = addToolBar("main");
    tb->setMovable(false);
    auto *searchBox = new QLineEdit;
    searchBox->setPlaceholderText("Search…");
    searchBox->setMaximumWidth(200);
    tb->addWidget(searchBox);
    tb->addSeparator();
    tb->addAction(QIcon::fromTheme("edit-copy"), "Copy Details");

    auto *splitter = new QSplitter(Qt::Horizontal);

    auto *sidebar = new QListWidget;
    sidebar->setMaximumWidth(265);
    sidebar->setMinimumWidth(200);
    sidebar->setFrameShape(QFrame::NoFrame);

    addGroup(sidebar, "Basic Information");
    addItem(sidebar, "system-run",                  "About this System");
    addItem(sidebar, "utilities-system-monitor",    "System Monitor");
    addItem(sidebar, "battery",                     "Energy");
    addGroup(sidebar, "Network");
    addItem(sidebar, "network-wired",               "Network Interfaces");
    addItem(sidebar, "network-server",              "Samba Status");
    addGroup(sidebar, "Devices");
    addItem(sidebar, "audio-card",                  "Audio");
    addItem(sidebar, "drive-harddisk",              "Block Devices");
    addItem(sidebar, "cpu",                         "CPU");
    addItem(sidebar, "security-high",               "Firmware Security");
    addItem(sidebar, "cpu",                         "Interrupts");
    addItem(sidebar, "media-flash",                 "Memory");
    addItem(sidebar, "pci",                         "PCI");
    addItem(sidebar, "drive-harddisk",              "SMART Status");
    addItem(sidebar, "temperature-normal",          "Sensors");

    sidebar->setCurrentRow(1);

    m_stack = new QStackedWidget;
    m_stack->addWidget(createAboutPage());
    m_stack->addWidget(createPaymentPage());

    splitter->addWidget(sidebar);
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    setCentralWidget(splitter);
    statusBar()->showMessage(
        "Arch Linux (Free)  —  Upgrade your plan to unlock the full potential of your OS");
}

// ── System info readers ──────────────────────────────────────────────────────

QString MainWindow::readSysFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return "Unknown";
    return QString::fromUtf8(f.readAll()).trimmed();
}

QString MainWindow::getCpuInfo() {
    QFile f("/proc/cpuinfo");
    if (!f.open(QIODevice::ReadOnly)) return "Unknown";
    int count = 0;
    QString model;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.startsWith("processor")) ++count;
        if (line.startsWith("model name") && model.isEmpty())
            model = line.section(':', 1).trimmed();
    }
    return QString("%1 × %2").arg(count).arg(model);
}

QString MainWindow::getMemInfo() {
    QFile f("/proc/meminfo");
    if (!f.open(QIODevice::ReadOnly)) return "Unknown";
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.startsWith("MemTotal:")) {
            const long kb = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(1).toLong();
            return QString("%1 GiB of RAM").arg(qRound(kb / 1024.0 / 1024.0));
        }
    }
    return "Unknown";
}

QString MainWindow::getKernelVersion() {
    QProcess p; p.start("uname", {"-r"}); p.waitForFinished(3000);
    const QString v = p.readAllStandardOutput().trimmed();
    return v.isEmpty() ? "Unknown" : v + " (64-bit)";
}

QString MainWindow::getGraphicsPlatform() {
    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) return "Wayland";
    if (!qgetenv("DISPLAY").isEmpty())         return "X11";
    return "Unknown";
}

QString MainWindow::getKdePlasmaVersion() {
    QProcess p; p.start("plasmashell", {"--version"}); p.waitForFinished(3000);
    const QString out = p.readAllStandardOutput().trimmed();
    return out.isEmpty() ? "Unknown" : out.split(' ').last();
}

QString MainWindow::getGraphicsInfo() {
    QProcess p;
    p.start("bash", {"-c",
        "lspci 2>/dev/null | grep -Ei 'VGA|3D|Display' | head -1 "
        "| sed 's/.*: //' | sed 's/ (rev.*//' | sed 's/Corporation //' "});
    p.waitForFinished(3000);
    const QString out = p.readAllStandardOutput().trimmed();
    return out.isEmpty() ? "Unknown" : out;
}

// ── About page ───────────────────────────────────────────────────────────────

QWidget *MainWindow::createAboutPage() {
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *root  = new QWidget;
    auto *vlay  = new QVBoxLayout(root);
    vlay->setContentsMargins(48, 32, 48, 32);
    vlay->setSpacing(18);

    // ── Logo + title ──
    auto *headerBox = new QHBoxLayout;
    headerBox->setSpacing(20);

    auto *logoLbl = new QLabel;
    QPixmap logo;
    for (const QString &p : {"/usr/share/pixmaps/archlinux-logo.png",
                               "/usr/share/pixmaps/archlinux-logo.svg"})
        if (logo.load(p)) break;

    if (!logo.isNull())
        logoLbl->setPixmap(logo.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logoLbl->setText("🐧"), logoLbl->setStyleSheet("font-size:64px;");

    auto *titleVlay = new QVBoxLayout;
    titleVlay->setSpacing(4);

    auto *titleLbl = new QLabel("Arch Linux <b>(Free)</b>");
    QFont tf = titleLbl->font(); tf.setPointSize(tf.pointSize() + 9); tf.setBold(true);
    titleLbl->setFont(tf);

    auto *linkLbl = new QLabel(R"(<a href="https://archlinux.org">https://archlinux.org</a>)");
    linkLbl->setOpenExternalLinks(true);

    titleVlay->addWidget(titleLbl);
    titleVlay->addWidget(linkLbl);
    titleVlay->addStretch();

    headerBox->addWidget(logoLbl);
    headerBox->addLayout(titleVlay);
    headerBox->addStretch();
    vlay->addLayout(headerBox);

    auto makeSep = [&]() -> QFrame* {
        auto *f = new QFrame; f->setFrameShape(QFrame::HLine); f->setFrameShadow(QFrame::Sunken);
        return f;
    };
    vlay->addWidget(makeSep());

    // ── Subscription section ──
    auto *planTitle = new QLabel("Choose Your Arch Linux Plan");
    QFont ptf = planTitle->font(); ptf.setBold(true); ptf.setPointSize(ptf.pointSize() + 2);
    planTitle->setFont(ptf);
    planTitle->setAlignment(Qt::AlignCenter);
    vlay->addWidget(planTitle);

    auto *subLine = new QLabel("Unlock the full Arch experience. Limited-time offer — prices may change without notice.");
    subLine->setAlignment(Qt::AlignCenter);
    subLine->setStyleSheet("color:gray;");
    vlay->addWidget(subLine);

    m_tierGroup = new QButtonGroup(this);

    const QStringList f1 = {"✓ Arch Linux access",    "✓ Rolling updates",
                              "✓ Pacman",               "✗ AI Assistant",
                              "✗ Hyprland access",      "✗ Priority support"};
    const QStringList f2 = {"✓ Arch Linux access",    "✓ Rolling updates",
                              "✓ Pacman",               "✓ AI Assistant",
                              "✗ Hyprland access",      "✗ Priority support"};
    const QStringList f3 = {"✓ Arch Linux access",    "✓ Rolling updates",
                              "✓ Pacman",               "✓ AI Assistant",
                              "✓ Hyprland access",      "✓ Priority support"};

    auto *r0 = new QRadioButton; r0->setChecked(true); m_tierGroup->addButton(r0, 0);
    auto *r1 = new QRadioButton;                        m_tierGroup->addButton(r1, 1);
    auto *r2 = new QRadioButton;                        m_tierGroup->addButton(r2, 2);

    auto *tiersRow = new QHBoxLayout; tiersRow->setSpacing(12);
    tiersRow->addWidget(makeTierCard("Arch (Free)",      "$7.99",  f1, "⭐ BEST VALUE", r0));
    tiersRow->addWidget(makeTierCard("Arch Pro",         "$39.99", f2, "",              r1));
    tiersRow->addWidget(makeTierCard("Arch Enterprise",  "$79.99", f3, "🏆 MOST FEATURES", r2));
    vlay->addLayout(tiersRow);

    auto *subBtn = new QPushButton("Subscribe Now  →");
    subBtn->setMinimumHeight(40);
    QFont sbf = subBtn->font(); sbf.setBold(true); subBtn->setFont(sbf);
    subBtn->setStyleSheet(
        "QPushButton{background:#1793d1;color:white;border-radius:6px;padding:8px 24px;}"
        "QPushButton:hover{background:#1565a8;}");
    connect(subBtn, &QPushButton::clicked, this, &MainWindow::onSubscribeClicked);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch(); btnRow->addWidget(subBtn); btnRow->addStretch();
    vlay->addLayout(btnRow);

    auto *disclaimer = new QLabel(
        "* By subscribing you agree to our Terms of Service, Privacy Policy, Cookie Policy, EULA, "
        "Data Resale Agreement, and the existential condition of using a rolling-release distro.");
    disclaimer->setStyleSheet("color:gray;font-size:9px;");
    disclaimer->setWordWrap(true);
    disclaimer->setAlignment(Qt::AlignCenter);
    vlay->addWidget(disclaimer);

    vlay->addWidget(makeSep());

    // ── Software section ──
    auto sectionLabel = [&](const QString &t) {
        auto *l = new QLabel(t);
        QFont f = l->font(); f.setBold(true); f.setPointSize(f.pointSize() + 2);
        l->setFont(f); l->setAlignment(Qt::AlignCenter); return l;
    };

    vlay->addWidget(sectionLabel("Software"));

    auto *softForm = new QFormLayout;
    softForm->setLabelAlignment(Qt::AlignRight);
    softForm->setHorizontalSpacing(24); softForm->setVerticalSpacing(6);
    softForm->addRow("KDE Plasma Version:", new QLabel(getKdePlasmaVersion()));
    softForm->addRow("Qt Version:",         new QLabel(QT_VERSION_STR));
    softForm->addRow("Kernel Version:",     new QLabel(getKernelVersion()));
    softForm->addRow("Graphics Platform:",  new QLabel(getGraphicsPlatform()));
    vlay->addLayout(softForm);

    vlay->addWidget(makeSep());

    // ── Hardware section ──
    vlay->addWidget(sectionLabel("Hardware"));

    auto *hwForm = new QFormLayout;
    hwForm->setLabelAlignment(Qt::AlignRight);
    hwForm->setHorizontalSpacing(24); hwForm->setVerticalSpacing(6);
    hwForm->addRow("Processors:",         new QLabel(getCpuInfo()));
    hwForm->addRow("Memory:",             new QLabel(getMemInfo()));
    hwForm->addRow("Graphics Processor:", new QLabel(getGraphicsInfo()));
    hwForm->addRow("Manufacturer:",       new QLabel(readSysFile("/sys/class/dmi/id/sys_vendor")));
    hwForm->addRow("Product Name:",       new QLabel(readSysFile("/sys/class/dmi/id/product_name")));
    hwForm->addRow("System Version:",     new QLabel(readSysFile("/sys/class/dmi/id/product_version")));

    // Serial with Show button
    auto *serialRow = new QHBoxLayout;
    auto *serialHidden = new QLabel("••••••••••");
    auto *showBtn = new QPushButton(QIcon::fromTheme("view-visible"), "Show");
    showBtn->setFlat(true);
    const QString serial = readSysFile("/sys/class/dmi/id/product_serial");
    connect(showBtn, &QPushButton::clicked, [serialHidden, showBtn, serial]() {
        serialHidden->setText(serial); showBtn->setEnabled(false);
    });
    serialRow->addWidget(serialHidden); serialRow->addWidget(showBtn); serialRow->addStretch();
    hwForm->addRow("Serial Number:", serialRow);

    vlay->addLayout(hwForm);
    vlay->addStretch();

    scroll->setWidget(root);
    return scroll;
}

// ── Payment page ─────────────────────────────────────────────────────────────

QWidget *MainWindow::createPaymentPage() {
    auto *page = new QWidget;
    auto *vlay = new QVBoxLayout(page);
    vlay->setContentsMargins(80, 36, 80, 36);
    vlay->setSpacing(14);

    auto *backBtn = new QPushButton(QIcon::fromTheme("go-previous"), "Back");
    backBtn->setFlat(true);
    connect(backBtn, &QPushButton::clicked, this, &MainWindow::onPaymentBack);
    auto *backRow = new QHBoxLayout;
    backRow->addWidget(backBtn); backRow->addStretch();
    vlay->addLayout(backRow);

    auto *payTitle = new QLabel("Complete Your Purchase");
    QFont ptf = payTitle->font(); ptf.setBold(true); ptf.setPointSize(ptf.pointSize() + 4);
    payTitle->setFont(ptf); payTitle->setAlignment(Qt::AlignCenter);
    vlay->addWidget(payTitle);

    auto *secBadge = new QLabel("🔒  Secured by TrustShield™ Payment Gateway v0.1-alpha  |  PCI-DSS Compliant*");
    secBadge->setAlignment(Qt::AlignCenter);
    secBadge->setStyleSheet("color:green;font-size:11px;");
    vlay->addWidget(secBadge);

    vlay->addSpacing(8);

    auto *cardBox = new QGroupBox("Payment Details");
    auto *cardForm = new QFormLayout(cardBox);

    auto *cardNum = new QLineEdit;
    cardNum->setInputMask("9999 9999 9999 9999");
    cardNum->setText("1234123412341234");
    cardForm->addRow("Card Number:", cardNum);

    auto *expRow = new QHBoxLayout;
    auto *expiry = new QLineEdit;
    expiry->setInputMask("99/99"); expiry->setMaximumWidth(70);
    expiry->setText("1234");
    auto *cvv = new QLineEdit; cvv->setMaximumWidth(60);
    cvv->setEchoMode(QLineEdit::Password);
    cvv->setText("1234");
    expRow->addWidget(expiry);
    expRow->addSpacing(12);
    expRow->addWidget(new QLabel("CVV:")); expRow->addWidget(cvv);
    expRow->addStretch();
    cardForm->addRow("Expiry:", expRow);

    auto *cardName = new QLineEdit;
    cardName->setText("John Arch Linux Doe");
    cardForm->addRow("Name on Card:", cardName);

    auto *address = new QLineEdit;
    address->setText("1 Rolling Release Ave");
    cardForm->addRow("Billing Address:", address);

    auto *countryRow = new QHBoxLayout;
    auto *country = new QLineEdit; country->setMaximumWidth(55);
    country->setText("US");
    auto *zip = new QLineEdit; zip->setPlaceholderText("ZIP / Postal code");
    zip->setText("12345"); zip->setMaximumWidth(110);
    countryRow->addWidget(country);
    countryRow->addSpacing(8);
    countryRow->addWidget(new QLabel("ZIP:"));
    countryRow->addWidget(zip);
    countryRow->addStretch();
    cardForm->addRow("Country:", countryRow);

    vlay->addWidget(cardBox);

    m_payTotalLabel = new QLabel("Total: $7.99/month (+ applicable taxes)");
    m_payTotalLabel->setAlignment(Qt::AlignCenter);
    QFont tf = m_payTotalLabel->font(); tf.setBold(true); m_payTotalLabel->setFont(tf);
    vlay->addWidget(m_payTotalLabel);

    auto *feeNote = new QLabel(
        "* Prices shown exclude VAT/GST. Processing fee of $0.00 may apply. "
        "Subscription auto-renews until cancelled via certified mail.");
    feeNote->setStyleSheet("color:gray;font-size:9px;");
    feeNote->setWordWrap(true);
    feeNote->setAlignment(Qt::AlignCenter);
    vlay->addWidget(feeNote);

    m_payNowBtn = new QPushButton("PAY NOW");
    m_payNowBtn->setMinimumHeight(44);
    QFont pbf = m_payNowBtn->font(); pbf.setBold(true); pbf.setPointSize(pbf.pointSize() + 2);
    m_payNowBtn->setFont(pbf);
    m_payNowBtn->setStyleSheet(
        "QPushButton{background:#27ae60;color:white;border-radius:6px;}"
        "QPushButton:hover{background:#1e8449;}"
        "QPushButton:disabled{background:#7f8c8d;}");
    connect(m_payNowBtn, &QPushButton::clicked, this, &MainWindow::onPayNowClicked);
    vlay->addWidget(m_payNowBtn);

    m_payProgress = new QProgressBar;
    m_payProgress->setRange(0, 0);
    m_payProgress->setTextVisible(false);
    m_payProgress->setVisible(false);
    vlay->addWidget(m_payProgress);

    m_paymentError = new QLabel;
    m_paymentError->setWordWrap(true);
    m_paymentError->setAlignment(Qt::AlignCenter);
    m_paymentError->setStyleSheet("color:#e74c3c;font-family:monospace;font-size:11px;");
    m_paymentError->setVisible(false);
    vlay->addWidget(m_paymentError);

    m_paymentPunchline = new QLabel(
        "\"I guess this is what happens when you vibecode your payment processor.\"");
    m_paymentPunchline->setAlignment(Qt::AlignCenter);
    m_paymentPunchline->setStyleSheet("color:gray;font-style:italic;font-size:11px;margin-top:6px;");
    m_paymentPunchline->setWordWrap(true);
    m_paymentPunchline->setVisible(false);
    vlay->addWidget(m_paymentPunchline);

    vlay->addStretch();

    auto *footer = new QLabel(
        "By clicking PAY NOW you agree to be charged monthly until the heat death of the universe. "
        "Cancellation requires 30 days' written notice to our registered address in the Cayman Islands. "
        "* TrustShield™ is not a real payment processor.");
    footer->setStyleSheet("color:gray;font-size:9px;");
    footer->setWordWrap(true);
    footer->setAlignment(Qt::AlignCenter);
    vlay->addWidget(footer);

    return page;
}

// ── Slots ────────────────────────────────────────────────────────────────────

void MainWindow::onSubscribeClicked() {
    const int id = m_tierGroup->checkedId();
    const double prices[] = {7.99, 39.99, 79.99};
    const QString names[] = {"Arch (Free)", "Arch Pro", "Arch Enterprise"};
    m_payTotalLabel->setText(
        QString("Total: $%1/month — %2  (+ applicable taxes)")
            .arg(prices[id], 0, 'f', 2).arg(names[id]));

    m_paymentError->hide();
    m_paymentPunchline->hide();
    m_payProgress->hide();
    m_payNowBtn->setEnabled(true);
    m_payNowBtn->setText("PAY NOW");
    m_stack->setCurrentIndex(1);
}

void MainWindow::onPayNowClicked() {
    m_payNowBtn->setEnabled(false);
    m_payNowBtn->setText("Processing…");
    m_payProgress->show();
    m_paymentError->hide();
    m_paymentPunchline->hide();

    QTimer::singleShot(2800, this, [this]() {
        m_payProgress->hide();
        m_paymentError->setText(
            "❌  Error 500: Internal Server Error\n\n"
            "    TypeError: Cannot read properties of undefined (reading 'charge')\n"
            "        at PaymentProcessor.js:1:1\n"
            "        at stripe.processCard (payment-gateway.min.js:1:1)\n"
            "        at Object.<anonymous> (checkout.js:1:1)\n\n"
            "    Request ID: pay_undefined_NaN_NaN\n"
            "    Please try again later, or never.");
        m_paymentError->show();
        m_paymentPunchline->show();
        m_payNowBtn->setText("Try Again");
        m_payNowBtn->setEnabled(true);
    });
}

void MainWindow::onPaymentBack() {
    m_stack->setCurrentIndex(0);
}
