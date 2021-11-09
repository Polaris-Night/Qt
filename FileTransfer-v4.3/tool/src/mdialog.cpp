#include "mdialog.h"
#include <QTextBrowser>
#include <QVBoxLayout>

MDialog::MDialog(QWidget *parent)
    : QDialog(parent)
{
    browser = new QTextBrowser(this);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(browser);
    resize(1005, 500);
}

MDialog::~MDialog()
{
}

void MDialog::setText(const QString &text)
{
    browser->setText(text);
}
