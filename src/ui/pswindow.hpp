#ifdef QT_GUI 

#pragma once

#include <QDialog>

#include "../ps.hpp"
#include <unordered_map>

namespace Ui
{
    class PsWindow;
}

class PsWindow : public QDialog
{
    Q_OBJECT

private:
    std::unordered_map<std::string, infos> m_umap;
    Ui::PsWindow *m_ui;
    pid_t m_pid;
    Ps m_ps;

    void Conf_pidTable();
    void setPid(QString);
    void Set_pidTable();

public:
    explicit PsWindow(QWidget * = nullptr);
    virtual ~PsWindow();

    pid_t getPid();

private slots:
    void on_search_textEdited(const QString &);
    void on_pidTable_doubleClicked(const QModelIndex &);
};

#endif 