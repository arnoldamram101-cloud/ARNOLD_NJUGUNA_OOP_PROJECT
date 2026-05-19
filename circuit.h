#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QList>

class Component;

class Circuit {
public:
    Circuit();
    void addComponent(Component* comp);
    void removeComponent(Component* comp);

    bool validateLoopConnectivity();
    void solveNetwork(bool isSimulatingEnabled);

    double getTotalResistance() const;
    double getTotalVoltage() const;
    const QList<Component*>& getComponents() const;

private:
    QList<Component*> activeComponents;
};

#endif // CIRCUIT_H