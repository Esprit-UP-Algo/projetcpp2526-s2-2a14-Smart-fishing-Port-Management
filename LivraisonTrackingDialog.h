#ifndef LIVRAISONTRACKINGDIALOG_H
#define LIVRAISONTRACKINGDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

class MapWidget : public QWidget {
    Q_OBJECT
public:
    MapWidget(QWidget* parent) : QWidget(parent), m_parent(nullptr), m_isPanning(false) {}
    void setTrackingDialog(class LivraisonTrackingDialog* dlg) { m_parent = dlg; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    class LivraisonTrackingDialog* m_parent;
    QPoint m_lastMousePos;
    bool m_isPanning;
};

class LivraisonTrackingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LivraisonTrackingDialog(const QString& id, const QString& address, QWidget *parent = nullptr);
    ~LivraisonTrackingDialog();

private slots:
    void onGeocodeFinished(QNetworkReply* reply);
    void onRouteFinished(QNetworkReply* reply);
    void updateSimulation();
    void startTracking();
    void zoomIn();
    void zoomOut();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    friend class MapWidget;
    void renderMap(QWidget* viewport);
    void setupUi();
    void geocodeAddress(const QString& address);
    void fetchRoute(double destLon, double destLat);
    void downloadMap();
    void onTileDownloaded(QNetworkReply* reply);
    void applyPan(int dx, int dy);
    QPointF coordinateToPixel(double lon, double lat);

    struct TileInfo {
        int x, y, z;
        QPoint pos;
    };
    QList<TileInfo> m_pendingTiles;
    int m_tilesToDownload;
    int m_tilesDownloaded;
    int m_zoomLevel;
    int m_tileStartX, m_tileStartY;
    int m_tileCountX, m_tileCountY;

    QString m_id;
    QString m_address;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_simTimer;
    
    // Data
    QList<QPair<double, double>> m_routePoints;
    int m_currentPointIndex;
    QPixmap m_mapPixmap;
    bool m_mapReady = false;
    
    // Bounds for coordinate to pixel mapping
    double minLon, maxLon, minLat, maxLat;
    
    // UI elements
    QLabel* m_statusLabel;
    QLabel* m_infoLabel;
    QLabel* m_timeLabel;
    QWidget* m_mapArea;
    QPushButton* m_startBtn;
    QPushButton* m_zoomInBtn;
    QPushButton* m_zoomOutBtn;

    double m_totalDuration;
    double m_totalDistance;
    QDateTime m_startTime;
    qint64 m_simElapsed;
    QTimer* m_panDebounceTimer;
    
    // Constants
    const QString ORS_API_KEY = "5b3ce3597851110001cf6248d3e9142f9e424268a05fbbd72565432";
    const double PORT_LON = 11.0622;
    const double PORT_LAT = 35.5042;
};

#endif // LIVRAISONTRACKINGDIALOG_H
