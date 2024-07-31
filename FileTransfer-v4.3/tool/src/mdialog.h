#ifndef MDIALOG_H
#define MDIALOG_H

#include <QDialog>

class QTextBrowser;

class MDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MDialog(QWidget *parent = nullptr);
    ~MDialog();

    /**
     * @brief setText
     * @details 设置文本
     * @param text 文本内容
     */
    void setText(const QString &text);

private:
    QTextBrowser *browser;
};

#endif // MDIALOG_H
