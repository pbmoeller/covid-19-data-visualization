#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QMap>
#include <QWidget>
#include <QGraphicsScene>

class QComboBox;

class MapView;

class CovidDataTreeItem;

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    bool loadMapData(const CovidDataTreeItem &rootItem);
    void addScene(const CovidDataTreeItem &sceneItem);

    void recursiveAddGraphicsItemsToScene(QGraphicsScene *scene, const CovidDataTreeItem &sceneItem);

private:
    void initUi();
    void resetSceneMap();

public slots:
    void sceneSelectComboChanged(const QString &text);

signals:
    void pathItemDoubleClicked(CovidDataTreeItem *);

private:
    MapView *mapView;

    QMap<QString, QGraphicsScene*> scenes;

    QComboBox *sceneSelectCombo;
};

#endif // MAPWIDGET_H