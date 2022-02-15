#ifdef QT_GUI
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QClipboard>
#include <QAction>
#include <QMainWindow>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace Ui
{
    class MainWindow;
}

class HexEditor;
class ELFParser;
class QTableWidgetItem;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // The main window that is created by mainwindow.ui
    Ui::MainWindow *m_ui;
    static MainWindow *m_man;

    // The dialog window
    boost::scoped_ptr<QDialog> m_dialog;

    // All the allocated Table values
    boost::ptr_vector<QTableWidgetItem> m_tableItems;

    // All the allocated Tree values
    boost::ptr_vector<QTreeWidgetItem> m_treeItems;

    // The reusable copy action
    boost::scoped_ptr<QAction> m_copyAction;

    // The reusable ELF parser
    boost::scoped_ptr<ELFParser> m_parser;

    // The resuable Editor Hex
    boost::scoped_ptr<HexEditor> m_hex_editor;

    QString m_FileName;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void sectionSelected(QTableWidgetItem*, QTableWidgetItem*);
    void programSelected(QTableWidgetItem*, QTableWidgetItem*);
    void parser(QString filename);
    void rparser();
    void reset();
    void openFile();
    void overviewToClipboard();
    void on_aboutButton_triggered();
    void conf_buttons();
    void conf_tables();
    void on_closeButton_clicked();
    void on_hexButton_clicked();
    static void visibleOn();
};

#endif //! MAINWINDOW_H
#endif //! QT_GUI
