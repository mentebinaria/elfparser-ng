#ifdef QT_GUI

#pragma once 

#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QClipboard>
#include <QAction>
#include <QMainWindow>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

#include "QHexView-ng.hpp"

namespace Ui
{
class MainWindow;
}

class ELFParser;
class QTableWidgetItem;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
  Q_OBJECT

 private:
  // The main window that is created by mainwindow.ui
  Ui::MainWindow *m_ui;

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
  QHexView *m_HexEditor;

  // split window
  QVBoxLayout *m_layout;

  // path name
  QString m_FileName;

  // entropy config 
  double m_Entropy;
  double m_VEntropy;
 public:
  explicit MainWindow ( QWidget *parent = 0 );
  ~MainWindow();


 public slots:
  void openFile();
  void parser ( QString filename );

  void sectionSelected ( QTableWidgetItem *, QTableWidgetItem * );
  void programSelected ( QTableWidgetItem *, QTableWidgetItem * );

  void on_gotoOffsetButton_triggered();
  void overviewToClipboard();
  void on_aboutButton_triggered();
  void on_reparseButton_triggered();
  void on_FullScreenButton_triggered();
  void conf_buttons();
  void conf_tables();
  void on_openButton_triggered();
  void on_EntroyLimitButton_triggered();
  void on_reportButton_triggered();
  void on_newButton_triggered();
  void on_helpButton_triggered();

};

#endif //! QT_GUI