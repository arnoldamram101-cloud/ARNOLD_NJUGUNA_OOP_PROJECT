#include "mainwindow.h"
#include "component.h"
#include <QInputDialog>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 1. Create the central workspace layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 2. Build the control sidebar panel
    QWidget *sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebar->setFixedWidth(220); // Widened slightly to fit diagnostics text neatly

    // NEW DIAGNOSTICS TEXT BOX
    telemetrySidebar = new QTextBrowser(this);
    telemetrySidebar->setMinimumHeight(180);
    telemetrySidebar->setStyleSheet("background-color: #1e1e1e; color: #00ffcc; font-family: monospace; font-size: 11px;");
    sidebarLayout->addWidget(telemetrySidebar);

    // CONTROL ACTIONS BUTTONS
    QPushButton *addBatteryBtn = new QPushButton("Add Battery", this);
    QPushButton *addResistorBtn = new QPushButton("Add Resistor", this);
    QPushButton *addLedBtn = new QPushButton("Add LED", this);
    QPushButton *addGroundBtn = new QPushButton("Add Ground", this);
    QPushButton *deleteBtn = new QPushButton("Delete Selected", this);
    QPushButton *changeValueBtn = new QPushButton("Edit Properties", this);

    // START SIMULATION BUTTON
    simulationToggleBtn = new QPushButton("Start Simulation", this);
    simulationToggleBtn->setMinimumHeight(35);
    // Make the text bold so it stands out
    QFont btnFont = simulationToggleBtn->font();
    btnFont.setBold(true);
    simulationToggleBtn->setFont(btnFont);

    sidebarLayout->addWidget(addBatteryBtn);
    sidebarLayout->addWidget(addResistorBtn);
    sidebarLayout->addWidget(addLedBtn);
    sidebarLayout->addWidget(addGroundBtn);
    sidebarLayout->addWidget(deleteBtn);
    sidebarLayout->addWidget(changeValueBtn);
    sidebarLayout->addWidget(simulationToggleBtn);
    sidebarLayout->addStretch();

    // 3. Set up the vector canvas scene
    scene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(scene, this);
    graphicsView->setFocusPolicy(Qt::ClickFocus);

    // 4. Combine elements into the main window view
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(graphicsView);

    // 5. Build the status bar down at the bottom
    customStatusBar = new QStatusBar(this);
    setStatusBar(customStatusBar);

    // 6. Connect C++ Signals to UI button actions manually
    connect(addBatteryBtn, &QPushButton::clicked, this, &MainWindow::on_addBatteryBtn_clicked);
    connect(addResistorBtn, &QPushButton::clicked, this, &MainWindow::on_addResistorBtn_clicked);
    connect(addLedBtn, &QPushButton::clicked, this, &MainWindow::on_addLedBtn_clicked);
    connect(addGroundBtn, &QPushButton::clicked, this, &MainWindow::on_addGroundBtn_clicked);
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::on_deleteBtn_clicked);
    connect(changeValueBtn, &QPushButton::clicked, this, &MainWindow::on_changeValueBtn_clicked);
    connect(simulationToggleBtn, &QPushButton::clicked, this, &MainWindow::on_simulationToggle_clicked);

    resize(1000, 650);
    updateCalculationLoop();
}

MainWindow::~MainWindow() {}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_R) {
        QList<QGraphicsItem*> selected = scene->selectedItems();
        for (QGraphicsItem* item : selected) {
            Component* comp = qgraphicsitem_cast<Component*>(item);
            if (comp) {
                comp->setRotation(comp->rotation() + 90.0);
                comp->update();
            }
        }
        updateCalculationLoop();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::on_addBatteryBtn_clicked() {
    Battery* b = new Battery("B" + QString::number(coreEngine.getComponents().size() + 1));
    scene->addItem(b); coreEngine.addComponent(b);
    updateCalculationLoop();
}

void MainWindow::on_addResistorBtn_clicked() {
    Resistor* r = new Resistor("R" + QString::number(coreEngine.getComponents().size() + 1));
    scene->addItem(r); coreEngine.addComponent(r);
    updateCalculationLoop();
}

void MainWindow::on_addLedBtn_clicked() {
    LED* led = new LED("LED" + QString::number(coreEngine.getComponents().size() + 1));
    scene->addItem(led); coreEngine.addComponent(led);
    updateCalculationLoop();
}

void MainWindow::on_addGroundBtn_clicked() {
    Ground* g = new Ground("GND");
    scene->addItem(g); coreEngine.addComponent(g);
    updateCalculationLoop();
}

void MainWindow::on_deleteBtn_clicked() {
    QList<QGraphicsItem*> selected = scene->selectedItems();
    for (QGraphicsItem* item : selected) {
        Component* comp = qgraphicsitem_cast<Component*>(item);
        if (comp) {
            coreEngine.removeComponent(comp);
            scene->removeItem(item);
            delete comp;
        }
    }
    updateCalculationLoop();
}

void MainWindow::on_changeValueBtn_clicked() {
    QList<QGraphicsItem*> selected = scene->selectedItems();
    if (selected.isEmpty()) {
        customStatusBar->showMessage("Select an element to adjust properties.", 2500);
        return;
    }

    Component* comp = qgraphicsitem_cast<Component*>(selected.first());
    if (!comp) return;

    bool ok;
    QString title = "Properties Manager: " + comp->name;

    if (comp->type == Component::TYPE_BATTERY) {
        double newValue = QInputDialog::getDouble(this, title, "Voltage (V):", comp->voltage, 0, 240, 1, &ok);
        if (ok) comp->voltage = newValue;
    }
    else if (comp->type == Component::TYPE_RESISTOR) {
        double newValue = QInputDialog::getDouble(this, title, "Resistance (Ω):", comp->resistance, 0.1, 1000000, 1, &ok);
        if (ok) comp->resistance = newValue;
    }
    else if (comp->type == Component::TYPE_LED) {
        double newValue = QInputDialog::getDouble(this, title, "Diode Resistance (Ω):", comp->resistance, 1.0, 1000, 1, &ok);
        if (ok) comp->resistance = newValue;
    }

    updateCalculationLoop();
}

void MainWindow::on_simulationToggle_clicked() {
    isSimulatingActive = !isSimulatingActive;
    updateCalculationLoop();
}

void MainWindow::updateCalculationLoop() {
    coreEngine.solveNetwork(isSimulatingActive);
    scene->update();

    // Update the Start Simulation button state and colors dynamically
    if (isSimulatingActive) {
        simulationToggleBtn->setText("Stop Simulation");
        simulationToggleBtn->setStyleSheet("background-color: #2eb872; color: white; border-radius: 4px;");
    } else {
        simulationToggleBtn->setText("Start Simulation");
        simulationToggleBtn->setStyleSheet("background-color: #e0e0e0; color: black; border-radius: 4px;");
    }

    // Pass structural validation tests to the sidebar renderer
    if (!isSimulatingActive) {
        syncTelemetrySidebar("PAUSED");
    } else if (!coreEngine.validateLoopConnectivity()) {
        syncTelemetrySidebar("LOOP BROKEN");
    } else {
        syncTelemetrySidebar("LIVE (RUNNING)");
    }
}

// RENDERS RICH HTML FORMATTED TELEMETRY DIRECTLY ONTO THE SIDEBAR
void MainWindow::syncTelemetrySidebar(QString statusLiteral) {
    QString html = "<html><body style='line-height:130%;'>";
    html += "<h3 style='margin:0 0 8px 0; color:#ffffff; border-bottom:1px solid #333; padding-bottom:3px;'>CIRCUIT LOGGER</h3>";

    // System Status Row
    QString statusColor = "#a0a0a0"; // Default paused gray
    if (statusLiteral == "LIVE (RUNNING)") statusColor = "#2eb872"; // Green
    else if (statusLiteral == "LOOP BROKEN") statusColor = "#ff4500"; // Red/Orange warning

    html += "<b>STATUS:</b> <span style='color:" + statusColor + ";'>" + statusLiteral + "</span><br><br>";

    if (statusLiteral == "LIVE (RUNNING)") {
        double totalI = 0.0;
        double netV = coreEngine.getTotalVoltage();
        double netR = coreEngine.getTotalResistance();
        bool hasBurntLed = false;

        QString componentRows = "";

        for (Component* c : coreEngine.getComponents()) {
            if (c->current > 0) totalI = c->current;

            // Build real-time metric strings for resistors and LEDs
            if (c->type == Component::TYPE_RESISTOR) {
                componentRows += QString("⚡ <b>%1:</b> %2V | %3A<br>").arg(c->name).arg(std::abs(c->voltageDrop), 0, 'f', 1).arg(c->current, 0, 'f', 3);
            }
            else if (c->type == Component::TYPE_LED) {
                if (c->isBurnt) {
                    hasBurntLed = true;
                    componentRows += QString("❌ <b>%1:</b> <span style='color:#ff4500;'>BURNT (OVERCURRENT)</span><br>").arg(c->name);
                } else {
                    componentRows += QString("💡 <b>%1:</b> GLOWING Active<br>").arg(c->name);
                }
            }
        }

        // Output structural telemetry metrics
        html += QString("<b>Net Voltage:</b> %1 V<br>").arg(netV, 0, 'f', 1);
        html += QString("<b>Total Load:</b> %1 Ω<br>").arg(netR, 0, 'f', 1);
        html += QString("<b>Loop Current:</b> %1 A<br>").arg(totalI, 0, 'f', 3);
        html += "<h4 style='margin:10px 0 4px 0; color:#e0e0e0;'>Live Elements:</h4>";
        html += componentRows;

        if (hasBurntLed) {
            html += "<p style='color:#ff4500; margin-top:10px; font-weight:bold;'>⚠️ CRITICAL ERROR: An LED has experienced overcurrent and failed!</p>";
        }

    } else if (statusLiteral == "LOOP BROKEN") {
        html += "<p style='color:#ff4500; font-weight:bold; margin:0;'>⚠️ WARNING: INCOMPLETE LOOP</p>";
        html += "<p style='color:#cccccc; margin-top:5px;'>The simulator cannot run because your circuit path is broken. Verify that:</p>";
        html += "<ul style='margin:5px 0; padding-left:15px; color:#cccccc;'>";
        html += "<li>Red connection terminal dots overlap closely (within 15px).</li>";
        html += "<li>At least one Ground (GND) reference element is anchored to the circuit.</li>";
        html += "</ul>";
    } else {
        html += "<p style='color:#a0a0a0; margin:0;'>Simulator is idle.</p>";
        html += "<p style='color:#888888; margin-top:5px;'>Click 'Start Simulation' with a valid closed circuit loop to view power metrics.</p>";
    }

    html += "</body></html>";

    // Print to sidebar box and fallback cleanly to the bottom status bar
    telemetrySidebar->setHtml(html);
    customStatusBar->showMessage("System State: [" + statusLiteral + "]");
}