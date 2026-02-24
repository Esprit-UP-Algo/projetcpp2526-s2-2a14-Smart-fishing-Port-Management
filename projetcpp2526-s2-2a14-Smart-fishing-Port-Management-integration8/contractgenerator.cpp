#include "contractgenerator.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QTextDocument>
#include <QPrinter>
#include <QPageSize>
#include <QMessageBox>

bool ContractGenerator::generateContract(const Quai& quai, const QString& clientName,
                                         const QString& clientCompany, const QString& duration,
                                         const QDate& startDate, QWidget* parent) {

    QString fileName = QFileDialog::getSaveFileName(parent,
                                                    "Enregistrer le contrat",
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                                                        "/Contrat_Quai_" + quai.getId() + "_" +
                                                        QDate::currentDate().toString("yyyyMMdd") + ".pdf",
                                                    "Fichiers PDF (*.pdf)");

    if (fileName.isEmpty()) return false;

    // Use QTextDocument for clean, reliable PDF generation
    QTextDocument doc;
    doc.setPageSize(QSizeF(595, 842)); // A4 in points

    QString contractNumber = QString("CT-%1-%2")
                                 .arg(quai.getId())
                                 .arg(QDate::currentDate().toString("yyyyMM"));

    QString html = QString(R"(
<html>
<head>
<style>
    body { font-family: 'Segoe UI', Arial, sans-serif; color: #1f2937; margin: 0; padding: 0; }
    .header {
        background: #2B5EA6;
        color: white;
        text-align: center;
        padding: 30px 20px 20px 20px;
        margin-bottom: 0;
    }
    .header h1 { margin: 0; font-size: 28pt; letter-spacing: 4px; }
    .header h3 { margin: 6px 0 0 0; font-size: 11pt; font-weight: normal; letter-spacing: 2px; }
    .contract-ref {
        background: #EFF6FF;
        border-left: 5px solid #2B5EA6;
        padding: 12px 20px;
        margin: 20px 0 10px 0;
        font-size: 10pt;
    }
    .contract-ref b { font-size: 13pt; color: #2B5EA6; }
    .section-title {
        color: #2B5EA6;
        font-size: 13pt;
        font-weight: bold;
        border-bottom: 2px solid #5D9CEC;
        padding-bottom: 4px;
        margin-top: 22px;
        margin-bottom: 10px;
    }
    table.info { width: 100%%; border-collapse: collapse; }
    table.info td { padding: 7px 10px; font-size: 10pt; }
    table.info td.label { font-weight: bold; color: #374151; width: 40%%; }
    table.info td.value { color: #1f2937; }
    .quai-card {
        background: #F0F7FF;
        border: 1px solid #5D9CEC;
        border-radius: 6px;
        padding: 14px;
        margin: 10px 0;
    }
    .tarif-box {
        background: #2B5EA6;
        color: white;
        text-align: center;
        padding: 16px;
        font-size: 16pt;
        font-weight: bold;
        border-radius: 6px;
        margin: 10px 0;
    }
    .terms { font-size: 9pt; color: #4b5563; line-height: 1.8; }
    .terms li { margin-bottom: 3px; }
    .signature-section { margin-top: 40px; }
    table.sig { width: 100%%; }
    table.sig td { text-align: center; padding: 10px; font-size: 9pt; color: #6b7280; }
    .sig-line { border-top: 1px solid #9ca3af; width: 180px; display: inline-block; margin-bottom: 6px; }
    .footer {
        text-align: center;
        font-size: 8pt;
        color: #9ca3af;
        border-top: 1px solid #e5e7eb;
        padding-top: 10px;
        margin-top: 30px;
    }
</style>
</head>
<body>

<div class="header">
    <h1>⚓ PORTFLOW</h1>
    <h3>CONTRAT DE LOCATION DE QUAI</h3>
</div>

<div class="contract-ref">
    Contrat N° : <b>%1</b> &nbsp;&nbsp;&nbsp;|&nbsp;&nbsp;&nbsp;
    Date d'émission : <b>%2</b>
</div>

<div class="section-title">INFORMATIONS CLIENT</div>
<table class="info">
    <tr><td class="label">Nom du client</td><td class="value">%3</td></tr>
    <tr><td class="label">Société / Organisation</td><td class="value">%4</td></tr>
    <tr><td class="label">Date de début</td><td class="value">%5</td></tr>
    <tr><td class="label">Durée du contrat</td><td class="value">%6</td></tr>
</table>

<div class="section-title">INFORMATIONS DU QUAI</div>
<div class="quai-card">
<table class="info">
    <tr><td class="label">Référence</td><td class="value">%7</td></tr>
    <tr><td class="label">Nom du quai</td><td class="value">%8</td></tr>
    <tr><td class="label">Capacité</td><td class="value">%9 emplacements</td></tr>
    <tr><td class="label">Taille maximale</td><td class="value">%10 mètres</td></tr>
    <tr><td class="label">Statut actuel</td><td class="value">%11</td></tr>
</table>
</div>

<div class="section-title">INFORMATIONS FINANCIÈRES</div>
<div class="tarif-box">TARIF JOURNALIER : %12 DT</div>

<div class="section-title">CONDITIONS GÉNÉRALES</div>
<div class="terms">
<ol>
    <li>Le locataire s'engage à utiliser le quai conformément à sa destination.</li>
    <li>Le paiement s'effectue mensuellement et d'avance.</li>
    <li>Le locataire est responsable des dommages causés au quai pendant la location.</li>
    <li>Le contrat peut être résilié avec un préavis de 15 jours.</li>
    <li>Le port se réserve le droit d'inspecter le quai à tout moment.</li>
    <li>Les horaires d'accès sont de 6h00 à 22h00.</li>
    <li>Le non-respect des règles de sécurité entraîne la résiliation immédiate.</li>
    <li>Le locataire doit souscrire une assurance responsabilité civile.</li>
</ol>
</div>

<div class="signature-section">
<table class="sig">
    <tr>
        <td>
            <div class="sig-line">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div><br>
            Signature du locataire<br><i>%3</i>
        </td>
        <td>
            <div class="sig-line">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</div><br>
            Signature du port<br><i>PortFlow Administration</i>
        </td>
    </tr>
</table>
</div>

<div class="footer">
    Document généré automatiquement par PortFlow — Contrat valable uniquement avec signature originale
</div>

</body>
</html>
    )")
                       .arg(contractNumber)                                          // %1
                       .arg(QDate::currentDate().toString("dd MMMM yyyy"))          // %2
                       .arg(clientName)                                              // %3
                       .arg(clientCompany)                                           // %4
                       .arg(startDate.toString("dd MMMM yyyy"))                     // %5
                       .arg(duration)                                                // %6
                       .arg(quai.getId())                                            // %7
                       .arg(quai.getNom())                                           // %8
                       .arg(quai.getCapacite())                                      // %9
                       .arg(quai.getTailleMax())                                     // %10
                       .arg(quai.getStatut())                                        // %11
                       .arg(quai.getTarif(), 0, 'f', 2);                            // %12

    doc.setHtml(html);
    doc.setPageSize(QSizeF(595, 842));

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    doc.print(&printer);

    QMessageBox::information(parent, "✅ Succès",
                             QString("Le contrat a été généré avec succès !\n📁 %1").arg(fileName));

    return true;
}
