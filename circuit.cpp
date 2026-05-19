#include "circuit.h"
#include "component.h"
#include <QLineF>

Circuit::Circuit() {}

void Circuit::addComponent(Component* comp) { activeComponents.append(comp); }
void Circuit::removeComponent(Component* comp) { activeComponents.removeOne(comp); }
//vector//
const QList<Component*>& Circuit::getComponents() const { return activeComponents; }
//connectivity checker//
bool Circuit::validateLoopConnectivity() {
    if (activeComponents.size() < 2) return false;

    bool groundAnchorExist = false;
    for (Component* c : activeComponents) {
        if (c->type == Component::TYPE_GROUND) groundAnchorExist = true;
    }
    if (!groundAnchorExist) return false;

    const double CONNECTOR_THRESHOLD = 15.0;
    int structuralMatches = 0;
//nested loops//
    for (int i = 0; i < activeComponents.size(); ++i) {
        Component* c1 = activeComponents[i];
        bool hasValidIntersection = false;

        for (int j = 0; j < activeComponents.size(); ++j) {
            if (i == j) continue;
            Component* c2 = activeComponents[j];

            double d00 = QLineF(c1->getAbsolutePin0(), c2->getAbsolutePin0()).length();
            double d01 = QLineF(c1->getAbsolutePin0(), c2->getAbsolutePin1()).length();
            double d10 = QLineF(c1->getAbsolutePin1(), c2->getAbsolutePin0()).length();
            double d11 = QLineF(c1->getAbsolutePin1(), c2->getAbsolutePin1()).length();

            if (d00 < CONNECTOR_THRESHOLD || d01 < CONNECTOR_THRESHOLD ||
                d10 < CONNECTOR_THRESHOLD || d11 < CONNECTOR_THRESHOLD) {
                hasValidIntersection = true;
            }
        }
        if (hasValidIntersection) structuralMatches++;
    }
    return (structuralMatches == activeComponents.size());
}

double Circuit::getTotalResistance() const {
    double totalR = 0.0;
    for (Component* c : activeComponents) totalR += c->resistance;
    return totalR;
}

double Circuit::getTotalVoltage() const {
    double totalV = 0.0;
    for (Component* c : activeComponents) {
        if (c->type == Component::TYPE_BATTERY) totalV += c->voltage;
    }
    return totalV;
}

void Circuit::solveNetwork(bool isSimulatingEnabled) {
    if (!isSimulatingEnabled || !validateLoopConnectivity()) {
        for (Component* c : activeComponents) {
            c->current = 0.0;
            c->voltageDrop = 0.0;
        }
        return;
    }

    double vNet = getTotalVoltage();
    double rNet = getTotalResistance();
    double sharedLoopCurrent = (rNet > 0.0) ? (vNet / rNet) : 0.0;

    for (Component* c : activeComponents) {
        c->updatePhysics(sharedLoopCurrent);
    }
}