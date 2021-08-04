#ifndef IMPORTPRIVATEKEYDIALOG_H
#define IMPORTPRIVATEKEYDIALOG_H

#include <QDialog>

namespace Ui {
    class ImportPrivateKeyDialog;
}
class AddressTableModel;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

/** Dialog for importing a private key
 */
class ImportPrivateKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportPrivateKeyDialog(QWidget *parent = 0);
    ~ImportPrivateKeyDialog();

    void setModel(AddressTableModel *model);

private slots:
    void on_ImportPrivateKeyPasteButton_clicked();

public slots:
    void accept();

private:
    bool save();

    Ui::ImportPrivateKeyDialog *ui;
    QDataWidgetMapper *mapper;
    AddressTableModel *model;
};

#endif // IMPORTPRIVATEKEYDIALOG_H
