#include "peche.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QDate>
#include <QRegularExpression>
#include "SmtpClient.h"

QString Peche::lastError = "";

Peche::Peche() {
    idLot = ""; reference = ""; espece = ""; quantiteKg = ""; dateCapture = ""; idBateau = ""; idFrigo = ""; idPecheur = "";
}

Peche::Peche(QString id, QString ref, QString esp, QString qte, QString date, QString idB, QString idF, QString idP) {
    this->idLot = id;
    this->reference = ref;
    this->espece = esp;
    this->quantiteKg = qte;
    this->dateCapture = date;
    this->idBateau = idB;
    this->idFrigo = idF;
    this->idPecheur = idP;
}

bool Peche::ajouter() {
    // Validation de base
    if (reference.trimmed().isEmpty()) {
        lastError = "La référence ne peut pas être vide.";
        return false;
    }

    // Validation du format (REF-YYYY-NOMBRE)
    QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
    if (!refRegex.match(reference).hasMatch()) {
        lastError = "Format de référence invalide (Attendu: REF-YYYY-NOMBRE).";
        return false;
    }

    // [NOUVEAU] Contrôle d'unicité
    if (referenceExiste(reference)) {
        lastError = "Cette référence existe déjà.";
        return false;
    }

    bool ok;
    double val = quantiteKg.toDouble(&ok);
    if (!ok || val <= 0) {
        lastError = "La quantité doit être un nombre positif.";
        return false;
    }

    QSqlQuery query;
    
    // [LOGIQUE INNOVANTE] Vérification du Quota Mensuel par Pêcheur (5000 Kg)
    QSqlQuery quotaQuery;
    quotaQuery.prepare("SELECT SUM(QUANTITE) FROM PECHES WHERE ID_PECHEUR = :idP AND TO_CHAR(DATECAPTURE, 'MM/YYYY') = :monthYear");
    quotaQuery.bindValue(":idP", idPecheur.toInt());
    quotaQuery.bindValue(":monthYear", QDate::currentDate().toString("MM/yyyy"));
    
    if (quotaQuery.exec() && quotaQuery.next()) {
        double currentTotal = quotaQuery.value(0).toDouble();
        double nextTotal = currentTotal + quantiteKg.toDouble();
        
        if (nextTotal > 5000.0) {
            qDebug() << "⚠️ ALERTE QUOTA : Dépassement de la limite mensuelle pour le pêcheur ID:" << idPecheur;
            
            // Récupération de l'email du pêcheur
            QSqlQuery emailQuery;
            emailQuery.prepare("SELECT EMAIL, PRENOM, NOM FROM EMPLOYEES WHERE ID_EMPLOYE = :id");
            emailQuery.bindValue(":id", idPecheur.toInt());
            
            if (emailQuery.exec() && emailQuery.next()) {
                QString targetEmail = emailQuery.value(0).toString();
                QString name = emailQuery.value(1).toString() + " " + emailQuery.value(2).toString();
                
                if (!targetEmail.isEmpty()) {
                    qDebug() << "Envoi d'un email à" << targetEmail << "pour dépassement de quota (" << nextTotal << "kg)";
                    SmtpClient smtp("dhafersmaoui8@gmail.com", "sjnn tpnr dfmf kuik");
                    QString subject = "⚠️ ALERTE PortFlow — Dépassement du Quota de Pêche";

                    QString month = QDate::currentDate().toString("MMMM yyyy");
                    QString body = QString(R"(
<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body style="margin:0;padding:0;background-color:#f0f4f8;font-family:'Segoe UI',Arial,sans-serif;">
  <table width="100%%" cellpadding="0" cellspacing="0" style="background:#f0f4f8;padding:40px 0;">
    <tr><td align="center">
      <table width="600" cellpadding="0" cellspacing="0" style="background:#ffffff;border-radius:16px;overflow:hidden;box-shadow:0 4px 24px rgba(0,0,0,0.10);">

        <!-- HEADER -->
        <tr>
          <td style="background:linear-gradient(135deg,#1e3a8a 0%%,#2563eb 100%%);padding:36px 40px 28px;text-align:center;">
            <div style="font-size:40px;margin-bottom:8px;">🚨</div>
            <h1 style="color:#ffffff;font-size:24px;margin:0 0 6px;letter-spacing:-0.5px;">ALERTE QUOTA DÉPASSÉ</h1>
            <p style="color:rgba(255,255,255,0.80);font-size:13px;margin:0;">Port Management System &mdash; PortFlow</p>
          </td>
        </tr>

        <!-- RED ALERT BANNER -->
        <tr>
          <td style="background:#dc2626;padding:14px 40px;text-align:center;">
            <p style="color:#fff;font-size:14px;font-weight:700;margin:0;letter-spacing:0.5px;">
              &#9888; QUOTA MENSUEL DÉPASSÉ &mdash; ACTION REQUISE
            </p>
          </td>
        </tr>

        <!-- BODY -->
        <tr>
          <td style="padding:36px 40px;">
            <p style="font-size:16px;color:#1e293b;margin:0 0 20px;">Bonjour <strong>%1</strong>,</p>
            <p style="font-size:14px;color:#475569;line-height:1.7;margin:0 0 28px;">
              Nous vous informons que votre quota de pêche autorisé pour le mois de
              <strong style="color:#1e3a8a;">%3</strong> a été dépassé.
              Veuillez prendre les mesures nécessaires immédiatement.
            </p>

            <!-- STATS CARDS -->
            <table width="100%%" cellpadding="0" cellspacing="0" style="margin-bottom:28px;">
              <tr>
                <td width="48%%" style="background:#fef2f2;border:1.5px solid #fecaca;border-radius:12px;padding:18px;text-align:center;">
                  <p style="font-size:11px;color:#991b1b;text-transform:uppercase;letter-spacing:1px;margin:0 0 6px;font-weight:700;">Total capturé</p>
                  <p style="font-size:28px;font-weight:700;color:#dc2626;margin:0;">%2 kg</p>
                </td>
                <td width="4%%"></td>
                <td width="48%%" style="background:#f0fdf4;border:1.5px solid #bbf7d0;border-radius:12px;padding:18px;text-align:center;">
                  <p style="font-size:11px;color:#166534;text-transform:uppercase;letter-spacing:1px;margin:0 0 6px;font-weight:700;">Quota autorisé</p>
                  <p style="font-size:28px;font-weight:700;color:#16a34a;margin:0;">5 000 kg</p>
                </td>
              </tr>
            </table>

            <!-- PROGRESS BAR -->
            <p style="font-size:12px;color:#64748b;margin:0 0 6px;font-weight:600;">Utilisation du quota</p>
            <div style="background:#e2e8f0;border-radius:999px;height:14px;overflow:hidden;margin-bottom:28px;">
              <div style="background:linear-gradient(90deg,#dc2626,#f97316);height:100%%;width:100%%;border-radius:999px;"></div>
            </div>

            <p style="font-size:14px;color:#475569;line-height:1.7;margin:0 0 24px;">
              Merci de contacter l'administration du port dans les plus brefs délais afin de
              <strong>régulariser votre situation</strong> et d'éviter toute sanction réglementaire.
            </p>

            <!-- INFO BOX -->
            <table width="100%%" cellpadding="0" cellspacing="0">
              <tr>
                <td style="background:#eff6ff;border-left:4px solid #3b82f6;border-radius:0 8px 8px 0;padding:14px 16px;">
                  <p style="font-size:13px;color:#1e40af;margin:0;">
                    📈 Ce système surveille automatiquement les quotas de chaque pêcheur.
                    Toute nouvelle capture sera bloquée jusqu'à la régularisation.
                  </p>
                </td>
              </tr>
            </table>
          </td>
        </tr>

        <!-- FOOTER -->
        <tr>
          <td style="background:#1e293b;padding:24px 40px;text-align:center;">
            <p style="color:rgba(255,255,255,0.9);font-size:15px;font-weight:700;margin:0 0 4px;">PortFlow</p>
            <p style="color:rgba(255,255,255,0.5);font-size:12px;margin:0;">Smart Fishing Port Management System</p>
            <p style="color:rgba(255,255,255,0.35);font-size:11px;margin:12px 0 0;">Cet email a été envoyé automatiquement. Ne pas répondre à ce message.</p>
          </td>
        </tr>

      </table>
    </td></tr>
  </table>
</body>
</html>
)").arg(name).arg(nextTotal).arg(month);

                    if (smtp.sendMail("dhafersmaoui8@gmail.com", targetEmail, subject, body)) {
                        qDebug() << "Email d'alerte envoyé avec succès à" << targetEmail;
                    } else {
                        qDebug() << "Erreur lors de l'envoi de l'email d'alerte.";
                    }
                }
            }
        }
    }

    query.prepare("INSERT INTO PECHES (IDLOT, REFERENCE, ESPECE, QUANTITE, DATECAPTURE, IDBATEAU, IDFRIGO, ID_PECHEUR) "
                  "VALUES (:id, :ref, :esp, :qte, :date, :idb, :idf, :idp)");

    int idNum = idLot.startsWith("LOT") ? idLot.mid(3).toInt() : idLot.toInt();
    query.bindValue(":id", idNum);
    query.bindValue(":ref", reference);
    query.bindValue(":esp", espece);
    query.bindValue(":qte", quantiteKg.toDouble());
    
    QDate qdate = QDate::fromString(dateCapture, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(dateCapture, Qt::ISODate);
    query.bindValue(":date", qdate);

    query.bindValue(":idb", idBateau);
    query.bindValue(":idf", idFrigo);
    query.bindValue(":idp", idPecheur.toInt());

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    
    // Mettre à jour l'occupation du frigo
    if (!idFrigo.isEmpty()) {
        QSqlQuery updateFrigo;
        updateFrigo.prepare("UPDATE FRIGOS SET OCCUPATION = GREATEST(0, NVL(OCCUPATION, 0) + :qte) WHERE IDFRIGO = :idf");
        updateFrigo.bindValue(":qte", quantiteKg.toDouble());
        updateFrigo.bindValue(":idf", idFrigo);
        updateFrigo.exec();
    }
    
    query.finish();
    return true;
}

QSqlQueryModel* Peche::afficher() {
    QSqlQueryModel* model = new QSqlQueryModel();
    model->setQuery("SELECT 'LOT' || p.IDLOT as ID, p.REFERENCE as \"Référence\", "
                    "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
                    "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
                    "e.PRENOM || ' ' || e.NOM as \"Pêcheur\", "
                    "p.IDBATEAU, p.IDFRIGO, p.ID_PECHEUR "
                    "FROM PECHES p "
                    "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                    "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO "
                    "LEFT JOIN EMPLOYEES e ON p.ID_PECHEUR = e.ID_EMPLOYE");
    
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Référence"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Espèce"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Quantité"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Date"));
    model->setHeaderData(5, Qt::Horizontal, QObject::tr("Bateau"));
    model->setHeaderData(6, Qt::Horizontal, QObject::tr("Frigo"));
    model->setHeaderData(7, Qt::Horizontal, QObject::tr("Pêcheur"));

    return model;
}

bool Peche::supprimer(QString id) {
    int idNum = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();
    
    // Récupérer l'ID du frigo et la quantité avant suppression
    QString oldIdFrigo;
    double oldQte = 0;
    QSqlQuery getInfo;
    getInfo.prepare("SELECT IDFRIGO, QUANTITE FROM PECHES WHERE IDLOT = :id");
    getInfo.bindValue(":id", idNum);
    if (getInfo.exec() && getInfo.next()) {
        oldIdFrigo = getInfo.value(0).toString();
        oldQte = getInfo.value(1).toDouble();
    }

    QSqlQuery query;
    query.prepare("DELETE FROM PECHES WHERE IDLOT = :id");
    query.bindValue(":id", idNum);
    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        lastError = "Aucun lot trouvé avec l'ID: " + QString::number(idNum);
        return false;
    }
    
    // Mettre à jour l'occupation du frigo
    if (!oldIdFrigo.isEmpty()) {
        QSqlQuery updateFrigo;
        updateFrigo.prepare("UPDATE FRIGOS SET OCCUPATION = GREATEST(0, NVL(OCCUPATION, 0) - :qte) WHERE IDFRIGO = :idf");
        updateFrigo.bindValue(":qte", oldQte);
        updateFrigo.bindValue(":idf", oldIdFrigo);
        updateFrigo.exec();
    }
    
    query.finish();
    return true;
}

bool Peche::modifier(QString id) {
    // Validation de base
    if (reference.trimmed().isEmpty()) {
        lastError = "La référence ne peut pas être vide.";
        return false;
    }

    // Validation du format (REF-YYYY-NOMBRE)
    QRegularExpression refRegex("^REF-\\d{4}-\\d+$");
    if (!refRegex.match(reference).hasMatch()) {
        lastError = "Format de référence invalide (Attendu: REF-YYYY-NOMBRE).";
        return false;
    }

    // [NOUVEAU] Contrôle d'unicité (en excluant le lot actuel)
    int idNumForCheck = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();
    if (referenceExiste(reference, idNumForCheck)) {
        lastError = "Cette référence est déjà utilisée par un autre lot.";
        return false;
    }

    bool ok;
    double val = quantiteKg.toDouble(&ok);
    if (!ok || val <= 0) {
        lastError = "La quantité doit être un nombre positif.";
        return false;
    }

    int idNum = id.startsWith("LOT") ? id.mid(3).toInt() : id.toInt();

    // Récupérer l'ancien frigo et l'ancienne quantité avant modification
    QString oldIdFrigo;
    double oldQte = 0;
    QSqlQuery getInfo;
    getInfo.prepare("SELECT IDFRIGO, QUANTITE FROM PECHES WHERE IDLOT = :id");
    getInfo.bindValue(":id", idNum);
    if (getInfo.exec() && getInfo.next()) {
        oldIdFrigo = getInfo.value(0).toString();
        oldQte = getInfo.value(1).toDouble();
    }

    QSqlQuery query;
    query.prepare("UPDATE PECHES SET reference = :ref, espece = :esp, quantite = :qte, datecapture = :date, "
                  "idbateau = :idb, idfrigo = :idf, id_pecheur = :idp "
                  "WHERE IDLOT = :id");

    query.bindValue(":id", idNum);
    query.bindValue(":ref", reference);
    query.bindValue(":esp", espece);
    query.bindValue(":qte", quantiteKg.toDouble());
    
    QDate qdate = QDate::fromString(dateCapture, "dd/MM/yyyy");
    if (!qdate.isValid()) qdate = QDate::fromString(dateCapture, Qt::ISODate);
    query.bindValue(":date", qdate);

    query.bindValue(":idb", idBateau);
    query.bindValue(":idf", idFrigo);
    query.bindValue(":idp", idPecheur.toInt());

    if (!query.exec()) {
        lastError = "Execute failed: " + query.lastError().text();
        return false;
    }
    
    // Mettre à jour l'occupation du frigo
    if (oldIdFrigo == idFrigo) {
        // Même frigo, on ajuste la différence
        QSqlQuery updateFrigo;
        updateFrigo.prepare("UPDATE FRIGOS SET OCCUPATION = GREATEST(0, NVL(OCCUPATION, 0) - :oldqte + :newqte) WHERE IDFRIGO = :idf");
        updateFrigo.bindValue(":oldqte", oldQte);
        updateFrigo.bindValue(":newqte", quantiteKg.toDouble());
        updateFrigo.bindValue(":idf", idFrigo);
        updateFrigo.exec();
    } else {
        // Enlever de l'ancien frigo
        if (!oldIdFrigo.isEmpty()) {
            QSqlQuery updateOld;
            updateOld.prepare("UPDATE FRIGOS SET OCCUPATION = GREATEST(0, NVL(OCCUPATION, 0) - :qte) WHERE IDFRIGO = :idf");
            updateOld.bindValue(":qte", oldQte);
            updateOld.bindValue(":idf", oldIdFrigo);
            updateOld.exec();
        }
        // Ajouter au nouveau frigo
        if (!idFrigo.isEmpty()) {
            QSqlQuery updateNew;
            updateNew.prepare("UPDATE FRIGOS SET OCCUPATION = GREATEST(0, NVL(OCCUPATION, 0) + :qte) WHERE IDFRIGO = :idf");
            updateNew.bindValue(":qte", quantiteKg.toDouble());
            updateNew.bindValue(":idf", idFrigo);
            updateNew.exec();
        }
    }
    
    query.finish();
    return true;
}

QSqlQueryModel* Peche::trier(QString critere, QString ordre) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QString dbCritere = "p.IDLOT"; // default

    if (critere == "ID")           dbCritere = "p.IDLOT";
    else if (critere == "Référence") dbCritere = "p.REFERENCE";
    else if (critere == "Espèce" || critere == "Catégorie") dbCritere = "p.ESPECE";
    else if (critere == "Quantité")  dbCritere = "p.QUANTITE";
    else if (critere == "Date")      dbCritere = "p.DATECAPTURE";

    QString queryString = QString(
        "SELECT 'LOT' || p.IDLOT as ID, p.REFERENCE as \"Référence\", "
        "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
        "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
        "e.PRENOM || ' ' || e.NOM as \"Pêcheur\", "
        "p.IDBATEAU, p.IDFRIGO, p.ID_PECHEUR "
        "FROM PECHES p "
        "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
        "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO "
        "LEFT JOIN EMPLOYEES e ON p.ID_PECHEUR = e.ID_EMPLOYE "
        "ORDER BY %1 %2").arg(dbCritere, ordre);

    model->setQuery(queryString);
    return model;
}

QSqlQueryModel* Peche::rechercher(QString val) {
    QSqlQueryModel* model = new QSqlQueryModel();
    QSqlQuery query;
    
    // Recherche étendue : référence, espèce, quantité, bateau ou frigo
    query.prepare("SELECT 'LOT' || p.IDLOT as ID, p.REFERENCE as \"Référence\", "
                  "p.ESPECE as \"Espèce\", p.QUANTITE as \"Quantité\", p.DATECAPTURE as \"Date\", "
                  "b.NOMBATEAU as \"Bateau\", f.REFERENCE as \"Frigo\", "
                  "e.PRENOM || ' ' || e.NOM as \"Pêcheur\", "
                  "p.IDBATEAU, p.IDFRIGO, p.ID_PECHEUR "
                  "FROM PECHES p "
                  "LEFT JOIN BATEAUX b ON p.IDBATEAU = b.IDBATEAU "
                  "LEFT JOIN FRIGOS f ON p.IDFRIGO = f.IDFRIGO "
                  "LEFT JOIN EMPLOYEES e ON p.ID_PECHEUR = e.ID_EMPLOYE "
                  "WHERE LOWER(p.REFERENCE) LIKE LOWER(:val) "
                  "OR LOWER(p.ESPECE) LIKE LOWER(:val) "
                  "OR LOWER(b.NOMBATEAU) LIKE LOWER(:val) "
                  "OR LOWER(f.REFERENCE) LIKE LOWER(:val) "
                  "OR TO_CHAR(p.QUANTITE) LIKE :val");
                  
    query.bindValue(":val", "%" + val + "%");
    query.exec();
    model->setQuery(std::move(query));
    return model;
}

bool Peche::referenceExiste(QString ref, int idLotExclu) {
    QSqlQuery query;
    if (idLotExclu == -1) {
        query.prepare("SELECT COUNT(*) FROM PECHES WHERE REFERENCE = :ref");
    } else {
        query.prepare("SELECT COUNT(*) FROM PECHES WHERE REFERENCE = :ref AND IDLOT != :id");
        query.bindValue(":id", idLotExclu);
    }
    query.bindValue(":ref", ref);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
