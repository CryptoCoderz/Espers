#ifndef TOKENUI_H
#define TOKENUI_H

#include <QDialog>
#include <QImage>

namespace Ui {
    class TokenUI;
}
class OptionsModel;

class TokenUI : public QDialog
{
    Q_OBJECT

public:
    explicit TokenUI(QWidget *parent = 0);
    ~TokenUI();

    void setModel(OptionsModel *model);

private slots:
    void on_lnReqAmount_textChanged();
    void on_btnSaveAs_clicked();

private:
    Ui::TokenUI *ui;
    OptionsModel *model;
};

#endif // TOKENUI_H
