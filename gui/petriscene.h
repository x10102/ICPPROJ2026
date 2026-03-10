#ifndef PETRISCENE_H
#define PETRISCENE_H

#include <QGraphicsScene>
#include "items.h"

enum class Tool { Select, AddPlace, AddTransition, AddArc};

class PetriScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit PetriScene(QObject *parent = nullptr);
    void setTool(Tool tool);
    Tool currentTool() const {return m_tool;}

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Tool m_tool = Tool::Select;
};

#endif // PETRISCENE_H
