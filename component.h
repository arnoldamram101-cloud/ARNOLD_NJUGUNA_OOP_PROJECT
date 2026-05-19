#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QString>
#include <QPainterPath>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMainWindow>
#include <cmath>

class Component : public QGraphicsObject {
    Q_OBJECT
public:
    enum ComponentType { TYPE_BATTERY, TYPE_RESISTOR, TYPE_LED, TYPE_GROUND };
    ComponentType type;
    QString name;

    double voltage = 0.0;
    double resistance = 0.0;
    double voltageDrop = 0.0;
    double current = 0.0;
    bool isBurnt = false;

    QPointF localPin0;
    QPointF localPin1;

    Component(ComponentType t, QString n) : type(t), name(n) {
        setFlags(QGraphicsItem::ItemIsMovable |
                 QGraphicsItem::ItemIsSelectable |
                 QGraphicsItem::ItemIsFocusable);
    }

    QPointF getAbsolutePin0() const { return mapToScene(localPin0); }
    QPointF getAbsolutePin1() const { return mapToScene(localPin1); }

    virtual void updatePhysics(double loopCurrent) = 0;
    QRectF boundingRect() const override { return QRectF(-45, -45, 90, 95); }

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override {
        Q_UNUSED(event);
        if (scene() && !scene()->views().isEmpty()) {
            QWidget *parentWidget = scene()->views().first()->window();
            QMainWindow *mainWindow = qobject_cast<QMainWindow*>(parentWidget);

            if (mainWindow) {
                this->setSelected(true);
                QMetaObject::invokeMethod(mainWindow, "on_changeValueBtn_clicked");
            }
        }
    }

    void drawPinDots(QPainter *painter) const {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(220, 50, 50));
        painter->drawEllipse(localPin0, 4, 4);
        painter->drawEllipse(localPin1, 4, 4);
    }

    
    void applyAntiOverlapTextTransform(QPainter *painter, double defaultX, double defaultY) const {
        painter->save();
        painter->translate(defaultX, defaultY);
        painter->rotate(-this->rotation()); 
    }
};

//  COLOURFUL BATTERY//
class Battery : public Component {
    Q_OBJECT
public:
    Battery(QString n, double v = 12.0) : Component(TYPE_BATTERY, n) {
        this->voltage = v;
        this->localPin0 = QPointF(0, -30);
        this->localPin1 = QPointF(0, 30);
    }
    void updatePhysics(double loopCurrent) override {
        this->current = loopCurrent;
        this->voltageDrop = -this->voltage;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);
        if (isSelected()) {
            painter->setPen(QPen(Qt::darkCyan, 1, Qt::DashLine));
            painter->drawRect(-35, -35, 70, 70);
        }
        painter->setPen(QPen(QColor(240, 60, 60), 4)); painter->drawLine(-25, -10, 25, -10);
        painter->setPen(QPen(QColor(60, 100, 240), 4)); painter->drawLine(-15, 0, 15, 0);
        painter->setPen(QPen(QColor(240, 60, 60), 4)); painter->drawLine(-25, 10, 25, 10);
        painter->setPen(QPen(QColor(60, 100, 240), 4)); painter->drawLine(-15, 20, 15, 20);
        painter->setPen(QPen(Qt::black, 2.5));
        painter->drawLine(0, -30, 0, -10); painter->drawLine(0, 20, 0, 30);

        
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->setPen(QPen(Qt::black, 1));

        applyAntiOverlapTextTransform(painter, -42, -15);
        painter->drawText(0, 0, name);
        painter->restore();

        applyAntiOverlapTextTransform(painter, 18, 25);
        painter->drawText(0, 0, QString::number(voltage) + "V");
        painter->restore();

        drawPinDots(painter);
    }
};

// COLOURFUL RESISTOR //
class Resistor : public Component {
    Q_OBJECT
public:
    Resistor(QString n, double r = 100.0) : Component(TYPE_RESISTOR, n) {
        this->resistance = r;
        this->localPin0 = QPointF(-30, 0);
        this->localPin1 = QPointF(30, 0);
    }
    void updatePhysics(double loopCurrent) override {
        this->current = loopCurrent;
        this->voltageDrop = loopCurrent * this->resistance;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);
        if (isSelected()) {
            painter->setPen(QPen(Qt::darkCyan, 1, Qt::DashLine));
            painter->drawRect(-35, -35, 70, 70);
        }
        painter->setPen(QPen(QColor(46, 184, 114), 3));
        QPainterPath path;
        path.moveTo(-30, 0); path.lineTo(-20, 0);
        path.lineTo(-15, -12); path.lineTo(-5, 12);  path.lineTo(5, -12);
        path.lineTo(15, 12);   path.lineTo(20, 0);    path.lineTo(30, 0);
        painter->drawPath(path);

        
        painter->setPen(QPen(Qt::black, 1));

        applyAntiOverlapTextTransform(painter, -20, -18);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(0, 0, name);
        painter->restore();

        applyAntiOverlapTextTransform(painter, -38, 24);
        painter->setFont(QFont("Arial", 7, QFont::Bold));
        painter->drawText(0, 0, "R: " + QString::number(resistance) + "R");
        painter->drawText(0, 10, "V: " + QString::number(std::abs(voltageDrop), 'f', 1) + "V");
        painter->drawText(0, 20, "I: " + QString::number(current, 'f', 3) + "A");
        painter->restore();

        drawPinDots(painter);
    }
};

//  COLOURFUL GROUND
class Ground : public Component {
    Q_OBJECT
public:
    Ground(QString n) : Component(TYPE_GROUND, n) {
        this->resistance = 0.0;
        this->localPin0 = QPointF(0, -30);
        this->localPin1 = QPointF(0, -30);
    }
    void updatePhysics(double) override { this->current = 0.0; this->voltageDrop = 0.0; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(QColor(100, 100, 100), 3));
        painter->drawLine(0, -30, 0, 0);
        painter->drawLine(-20, 0, 20, 0); painter->drawLine(-12, 6, 12, 6); painter->drawLine(-4, 12, 4, 12);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(220, 50, 50));
        painter->drawEllipse(localPin0, 4, 4);
    }
};

// DYNAMIC GLOWING LED 
class LED : public Component {
    Q_OBJECT
public:
    LED(QString n) : Component(TYPE_LED, n) {
    
        this->resistance = 400.0;
        this->localPin0 = QPointF(-30, 0);
        this->localPin1 = QPointF(30, 0);
    }
    void updatePhysics(double loopCurrent) override {
        this->current = loopCurrent;
        this->voltageDrop = loopCurrent * this->resistance;

        // Safety critical cap boundary set at 35mA overcurrent limits
        if (this->current > 0.035) this->isBurnt = true;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setRenderHint(QPainter::Antialiasing);
        if (isSelected()) {
            painter->setPen(QPen(Qt::darkCyan, 1, Qt::DashLine));
            painter->drawRect(-35, -35, 70, 70);
        }
        if (this->isBurnt) {
            painter->setPen(QPen(Qt::red, 2));

            applyAntiOverlapTextTransform(painter, -28, -22);
            painter->setFont(QFont("Arial", 8, QFont::Bold));
            painter->drawText(0, 0, "!! BURNT !!");
            painter->restore();

            painter->setBrush(QColor(90, 90, 90)); painter->setPen(QPen(Qt::darkGray, 3));
        } else {
            painter->setPen(QPen(Qt::black, 3));
            painter->setBrush(this->current > 0 ? QColor(255, 215, 0) : Qt::transparent);
        }
        painter->drawLine(-30, 0, -15, 0);
        static const QPointF triangle[3] = { QPointF(-15, 15), QPointF(15, 0), QPointF(-15, -15) };
        painter->drawPolygon(triangle, 3);
        painter->drawLine(15, 15, 15, -15); painter->drawLine(15, 0, 30, 0);

        if (this->current > 0 && !this->isBurnt) {
            painter->setPen(QPen(QColor(255, 69, 0), 2.5));
            painter->drawLine(-5, -18, 5, -28); painter->drawLine(5, -23, 5, -28);
            painter->drawLine(5, -18, 15, -28); painter->drawLine(15, -23, 15, -28);
        }

        painter->setPen(QPen(Qt::black, 1));

        applyAntiOverlapTextTransform(painter, -15, 28);
        painter->setFont(QFont("Arial", 8, QFont::Bold));
        painter->drawText(0, 0, name);
        painter->restore();

        drawPinDots(painter);
    }
};

#endif // COMPONENT_H
