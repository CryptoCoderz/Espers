#ifndef EDITCONFIGDIALOG_H
#define EDITCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
    class EditConfigDialog;
}
class ClientModel;

/** "About" dialog box */
class EditConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditConfigDialog(QWidget *parent = 0);
    ~EditConfigDialog();

    void setModel(ClientModel *model);
private:
    Ui::EditConfigDialog *ui;
    ClientModel *model;

public slots:
    void accept();
};

#endif // EDITCONFIGDIALOG_H
