#include "LivraisonTrackingDialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUrlQuery>
#include <QBuffer>
#include <QDateTime>
#include <cmath>

// MapWidget Methods
void MapWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
        m_isPanning = true;
        setCursor(Qt::ClosedHandCursor);
    }
}

void MapWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_isPanning && m_parent) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_lastMousePos = event->pos();
        m_parent->applyPan(delta.x(), delta.y());
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void MapWidget::paintEvent(QPaintEvent*) {
    if (m_parent) m_parent->renderMap(this);
}

LivraisonTrackingDialog::LivraisonTrackingDialog(const QString& id, const QString& address, QWidget *parent)
    : QDialog(parent), m_id(id), m_address(address), m_currentPointIndex(0), m_zoomLevel(0), m_simElapsed(0)
{
    setupUi();
    m_networkManager = new QNetworkAccessManager(this);
    m_simTimer = new QTimer(this);
    connect(m_simTimer, &QTimer::timeout, this, &LivraisonTrackingDialog::updateSimulation);
    
    m_panDebounceTimer = new QTimer(this);
    m_panDebounceTimer->setSingleShot(true);
    connect(m_panDebounceTimer, &QTimer::timeout, this, &LivraisonTrackingDialog::downloadMap);

    // Initial load from DB for persistence
    int idNum = m_id.startsWith("LIV") ? m_id.mid(3).toInt() : m_id.toInt();
    QSqlQuery query;
    query.prepare("SELECT DATE_DEPART, DUREE_ESTIMEE, STATUT FROM LIVRAISONS WHERE IDLIVRAISON = :id");
    query.bindValue(":id", idNum);
    if (query.exec() && query.next()) {
        QString currentStatus = query.value("STATUT").toString();
        m_startTime = query.value("DATE_DEPART").toDateTime();
        m_totalDuration = query.value("DUREE_ESTIMEE").toDouble();
        
        if (currentStatus == "En chemin" && m_startTime.isValid() && m_totalDuration > 0) {
            m_simElapsed = m_startTime.secsTo(QDateTime::currentDateTime());
            if (m_simElapsed < m_totalDuration) {
                m_simTimer->start(1000); 
                m_startBtn->setEnabled(false);
                m_statusLabel->setText("🚚 Reprise de la livraison...");
            } else {
                QSqlQuery update;
                update.prepare("UPDATE LIVRAISONS SET STATUT = 'Livré' WHERE IDLIVRAISON = :id");
                update.bindValue(":id", idNum);
                update.exec();
            }
        }
    }

    geocodeAddress(address);
}

LivraisonTrackingDialog::~LivraisonTrackingDialog() {}

void LivraisonTrackingDialog::setupUi()
{
    setWindowTitle("Suivi de Livraison - " + m_id);
    setMinimumSize(900, 750);
    setStyleSheet("background-color: #F8FAFC;");

    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    QFrame* header = new QFrame();
    header->setFixedHeight(80);
    header->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E3A8A, stop:1 #3B82F6);");
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(25, 0, 25, 0);

    QLabel* title = new QLabel("🚚 Suivi en Temps Réel: " + m_id);
    title->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    headerLay->addWidget(title);

    m_statusLabel = new QLabel("Chargement...");
    m_statusLabel->setStyleSheet("color: #E0F2FE; font-size: 14px;");
    headerLay->addWidget(m_statusLabel, 0, Qt::AlignRight);
    mainLay->addWidget(header);

    m_mapArea = new MapWidget(this);
    static_cast<MapWidget*>(m_mapArea)->setTrackingDialog(this);
    m_mapArea->setStyleSheet("background-color: #E2E8F0;");
    
    QVBoxLayout* zoomLay = new QVBoxLayout(m_mapArea);
    zoomLay->setContentsMargins(0, 0, 20, 0);
    zoomLay->setSpacing(10);
    zoomLay->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto makeZoomBtn = [&](const QString& label) {
        QPushButton* btn = new QPushButton(label);
        btn->setFixedSize(40, 40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("QPushButton { background: white; border: 1.5px solid #CBD5E1; border-radius: 20px; font-weight: bold; font-size: 18px; color: #1E3A8A; } QPushButton:hover { background: #F8FAFC; border: 1.5px solid #3B82F6; }");
        return btn;
    };
    m_zoomInBtn = makeZoomBtn("+");
    m_zoomOutBtn = makeZoomBtn("−");
    zoomLay->addWidget(m_zoomInBtn);
    zoomLay->addWidget(m_zoomOutBtn);
    connect(m_zoomInBtn, &QPushButton::clicked, this, &LivraisonTrackingDialog::zoomIn);
    connect(m_zoomOutBtn, &QPushButton::clicked, this, &LivraisonTrackingDialog::zoomOut);

    mainLay->addWidget(m_mapArea, 1);

    QFrame* controls = new QFrame();
    controls->setFixedHeight(110);
    controls->setStyleSheet("background: white; border-top: 1px solid #E2E8F0;");
    QHBoxLayout* ctrlLay = new QHBoxLayout(controls);
    ctrlLay->setContentsMargins(25, 10, 25, 10);

    QVBoxLayout* infoV = new QVBoxLayout();
    m_infoLabel = new QLabel("Destination: " + m_address);
    m_infoLabel->setStyleSheet("color: #334155; font-size: 14px; font-weight: 500;");
    infoV->addWidget(m_infoLabel);

    m_timeLabel = new QLabel("Calcul de l'itinéraire...");
    m_timeLabel->setStyleSheet("color: #2563EB; font-size: 13px; font-weight: 600; font-family: 'Segoe UI';");
    infoV->addWidget(m_timeLabel);

    QLabel* startLbl = new QLabel("Départ: Port de Mahdia");
    startLbl->setStyleSheet("color: #64748B; font-size: 12px;");
    infoV->addWidget(startLbl);
    ctrlLay->addLayout(infoV, 1);

    m_startBtn = new QPushButton("▶ Démarrer la Livraison");
    m_startBtn->setEnabled(false);
    m_startBtn->setFixedSize(220, 45);
    m_startBtn->setStyleSheet("QPushButton { background: #10B981; color: white; border-radius: 8px; font-weight: bold; font-size: 14px; } QPushButton:disabled { background: #CBD5E1; }");
    connect(m_startBtn, &QPushButton::clicked, this, &LivraisonTrackingDialog::startTracking);
    ctrlLay->addWidget(m_startBtn);

    mainLay->addWidget(controls);
}

void LivraisonTrackingDialog::applyPan(int dx, int dy)
{
    double lonRange = maxLon - minLon;
    double latRange = maxLat - minLat;
    double dLon = (double)dx / m_mapArea->width() * lonRange * -1.0;
    double dLat = (double)dy / m_mapArea->height() * latRange;
    minLon += dLon; maxLon += dLon;
    minLat += dLat; maxLat += dLat;
    m_panDebounceTimer->start(150);
    m_mapArea->update();
}

void LivraisonTrackingDialog::geocodeAddress(const QString& address)
{
    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery query;
    query.addQueryItem("q", address);
    query.addQueryItem("format", "json");
    query.addQueryItem("limit", "1");
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "PortFlow/1.0");
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onGeocodeFinished(reply); });
}

void LivraisonTrackingDialog::onGeocodeFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray results = doc.array();
        if (!results.isEmpty()) {
            QJsonObject first = results[0].toObject();
            fetchRoute(first["lon"].toString().toDouble(), first["lat"].toString().toDouble());
            return;
        }
    }
    m_statusLabel->setText("❌ Destination introuvable");
    reply->deleteLater();
}

void LivraisonTrackingDialog::fetchRoute(double destLon, double destLat)
{
    m_statusLabel->setText("🛣️ Recherche d'itinéraire...");
    QUrl url(QString("http://router.project-osrm.org/route/v1/driving/%1,%2;%3,%4")
             .arg(PORT_LON).arg(PORT_LAT).arg(destLon).arg(destLat));
    QUrlQuery query;
    query.addQueryItem("overview", "full");
    query.addQueryItem("geometries", "geojson");
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "PortFlow/1.0");
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onRouteFinished(reply); });
}

void LivraisonTrackingDialog::onRouteFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray routes = doc.object()["routes"].toArray();
        if (!routes.isEmpty()) {
            QJsonObject route = routes[0].toObject();
            QJsonArray coords = route["geometry"].toObject()["coordinates"].toArray();
            m_totalDistance = route["distance"].toDouble();
            m_totalDuration = route["duration"].toDouble();
            
            m_routePoints.clear();
            minLon = 180, maxLon = -180, minLat = 90, maxLat = -90;
            minLon = qMin(minLon, PORT_LON); maxLon = qMax(maxLon, PORT_LON);
            minLat = qMin(minLat, PORT_LAT); maxLat = qMax(maxLat, PORT_LAT);

            for (int i = 0; i < coords.size(); ++i) {
                QJsonArray p = coords[i].toArray();
                double lon = p[0].toDouble(); double lat = p[1].toDouble();
                m_routePoints.append({lon, lat});
                if (lon < minLon) minLon = lon; if (lon > maxLon) maxLon = lon;
                if (lat < minLat) minLat = lat; if (lat > maxLat) maxLat = lat;
            }
            downloadMap();
        }
    } else { m_statusLabel->setText("⚠️ Erreur Route"); }
    reply->deleteLater();
}

void LivraisonTrackingDialog::downloadMap()
{
    m_mapReady = false;
    m_statusLabel->setText("📥 Téléchargement de la carte...");
    auto l2t = [](double lon, int z) { return (int)(std::floor((lon + 180.0) / 360.0 * std::pow(2.0, z))); };
    auto lt2t = [](double lat, int z) { double r = lat * M_PI / 180.0; return (int)(std::floor((1.0 - std::log(std::tan(r) + 1.0 / std::cos(r)) / M_PI) / 2.0 * std::pow(2.0, z))); };

    if (m_zoomLevel == 0) {
        double diff = qMax(std::abs(maxLon - minLon), std::abs(maxLat - minLat));
        m_zoomLevel = (diff > 0.08) ? 11 : (diff > 0.04 ? 12 : (diff < 0.01 ? 14 : 13));
    }

    m_tileStartX = l2t(minLon, m_zoomLevel) - 1;
    m_tileStartY = lt2t(maxLat, m_zoomLevel) - 1;
    m_tileCountX = qBound(3, l2t(maxLon, m_zoomLevel) - m_tileStartX + 2, 8);
    m_tileCountY = qBound(3, lt2t(minLat, m_zoomLevel) - m_tileStartY + 2, 8);

    m_tilesToDownload = m_tileCountX * m_tileCountY;
    m_tilesDownloaded = 0;
    m_mapPixmap = QPixmap(m_tileCountX * 256, m_tileCountY * 256);
    m_mapPixmap.fill(QColor("#E2E8F0"));

    for (int x = 0; x < m_tileCountX; ++x) {
        for (int y = 0; y < m_tileCountY; ++y) {
            QUrl url(QString("https://a.basemaps.cartocdn.com/rastertiles/light_all/%1/%2/%3.png").arg(m_zoomLevel).arg(m_tileStartX + x).arg(m_tileStartY + y));
            QNetworkRequest request(url);
            request.setHeader(QNetworkRequest::UserAgentHeader, "PortFlow/1.0");
            QNetworkReply* reply = m_networkManager->get(request);
            reply->setProperty("tileX", x); reply->setProperty("tileY", y);
            connect(reply, &QNetworkReply::finished, this, [this, reply]() { onTileDownloaded(reply); });
        }
    }
}

void LivraisonTrackingDialog::onTileDownloaded(QNetworkReply* reply)
{
    m_tilesDownloaded++;
    if (reply->error() == QNetworkReply::NoError) {
        QPixmap tile; tile.loadFromData(reply->readAll());
        QPainter p(&m_mapPixmap); p.drawPixmap(reply->property("tileX").toInt() * 256, reply->property("tileY").toInt() * 256, tile);
    }
    if (m_tilesDownloaded >= m_tilesToDownload) {
        m_mapReady = true; m_statusLabel->setText("✅ Carte chargée"); m_startBtn->setEnabled(m_simTimer->isActive() ? false : true); m_mapArea->update();
    }
    reply->deleteLater();
}

QPointF LivraisonTrackingDialog::coordinateToPixel(double lon, double lat)
{
    auto l2px = [](double l, int z) { return (l + 180.0) / 360.0 * std::pow(2.0, z) * 256.0; };
    auto lt2px = [](double l, int z) { double r = l * M_PI / 180.0; return (1.0 - std::log(std::tan(r) + 1.0 / std::cos(r)) / M_PI) / 2.0 * std::pow(2.0, z) * 256.0; };
    return QPointF(l2px(lon, m_zoomLevel) - (m_tileStartX * 256.0), lt2px(lat, m_zoomLevel) - (m_tileStartY * 256.0));
}

void LivraisonTrackingDialog::renderMap(QWidget* viewport)
{
    QPainter painter(viewport);
    painter.setRenderHint(QPainter::Antialiasing);
    double offsetX = (viewport->width() - m_mapPixmap.width()) / 2.0;
    double offsetY = (viewport->height() - m_mapPixmap.height()) / 2.0;
    painter.drawPixmap(offsetX, offsetY, m_mapPixmap);
    auto tp = [&](double lon, double lat) { QPointF p = coordinateToPixel(lon, lat); return QPointF(offsetX + p.x(), offsetY + p.y()); };
    if (m_routePoints.size() > 1) {
        painter.setPen(QPen(QColor(37, 99, 235, 100), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        for (int i = 0; i < m_routePoints.size() - 1; ++i) painter.drawLine(tp(m_routePoints[i].first, m_routePoints[i].second), tp(m_routePoints[i+1].first, m_routePoints[i+1].second));
    }
    if (!m_routePoints.isEmpty()) {
        QPointF pos = tp(m_routePoints[m_currentPointIndex].first, m_routePoints[m_currentPointIndex].second);
        painter.setBrush(QColor("#10B981")); painter.setPen(QPen(Qt::white, 2)); painter.drawEllipse(pos, 10, 10);
        painter.setPen(Qt::black); painter.drawText(pos.x() + 12, pos.y() + 5, "🚚");
    }
}

void LivraisonTrackingDialog::startTracking()
{
    m_simElapsed = 0; m_startTime = QDateTime::currentDateTime(); m_simTimer->start(1000); m_startBtn->setEnabled(false); m_statusLabel->setText("🚚 Livraison lancée...");
    int idNum = m_id.startsWith("LIV") ? m_id.mid(3).toInt() : m_id.toInt();
    QSqlQuery q; q.prepare("UPDATE LIVRAISONS SET STATUT = 'En chemin', DATE_DEPART = :s, DUREE_ESTIMEE = :d WHERE IDLIVRAISON = :i");
    q.bindValue(":s", m_startTime); q.bindValue(":d", m_totalDuration); q.bindValue(":i", idNum); q.exec();
}

void LivraisonTrackingDialog::updateSimulation()
{
    if (m_routePoints.isEmpty() || m_totalDuration <= 0) return;
    m_simElapsed = m_startTime.secsTo(QDateTime::currentDateTime());
    double progress = qBound(0.0, (double)m_simElapsed / m_totalDuration, 1.0);
    m_currentPointIndex = qBound(0, (int)(progress * (m_routePoints.size() - 1)), m_routePoints.size() - 1);
    int rem = qMax(0, (int)(m_totalDuration - m_simElapsed));
    m_timeLabel->setText(QString("📏 ITINÉRAIRE: %1 km | ⏱️ ARRIVÉE DANS %2 MINS").arg(m_totalDistance/1000.0, 0, 'f', 1).arg(qMax(1, qRound(rem/60.0))));
    if (progress >= 1.0) {
        m_simTimer->stop(); m_statusLabel->setText("✅ Arrivé!");
        QSqlQuery q; q.prepare("UPDATE LIVRAISONS SET STATUT = 'Livré' WHERE IDLIVRAISON = :i");
        int idNum = m_id.startsWith("LIV") ? m_id.mid(3).toInt() : m_id.toInt(); q.bindValue(":i", idNum); q.exec();
    }
    m_mapArea->update();
}

void LivraisonTrackingDialog::zoomIn() { if (m_zoomLevel < 19) { m_zoomLevel++; downloadMap(); } }
void LivraisonTrackingDialog::zoomOut() { if (m_zoomLevel > 1) { m_zoomLevel--; downloadMap(); } }
void LivraisonTrackingDialog::paintEvent(QPaintEvent* event) { QDialog::paintEvent(event); }
