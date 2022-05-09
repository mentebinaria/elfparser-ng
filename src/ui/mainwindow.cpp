#ifdef QT_GUI
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "ui_about.h"
#include "inttablewidget.hpp"
#include "../elfparser.hpp"
#include "../abstract_sectionheader.hpp"
#include "../abstract_programheader.hpp"

#include <QInputDialog>
#include <QDesktopServices>
#include <QCursor>
#include <QInputDialog>
#include <QAbstractScrollArea>
#include <QClipboard>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <QCloseEvent>


MainWindow::MainWindow ( QWidget *parent ) : QMainWindow ( parent ),
  m_ui ( new Ui::MainWindow ),
  m_dialog(),
  m_tableItems(),
  m_treeItems(),
  m_copyAction(),
  m_parser(),
  m_HexEditor ( new QHexView ),
  m_layout ( new QVBoxLayout ),
  m_Entropy ( 7.0 )
{
  setWindowTitle ( "elfparser-ng" );
#if _WIN64 || _WIN32
  setWindowIcon ( QIcon ( "..\\..\\src\\ui\\assets\\bug.png" ) );
#else
  setWindowIcon ( QIcon ( "../src/ui/assets/bug.png" ) );
#endif
  m_ui->setupUi ( this );

  // create the copy action and apply signals as needed
  m_copyAction.reset ( new QAction ( tr ( "Copy" ), this ) );
  m_copyAction->setShortcut ( QKeySequence::Copy );
  connect ( m_copyAction.get(), SIGNAL ( triggered() ), this, SLOT ( overviewToClipboard() ) );

  // attach copy to widget
  m_ui->overviewTable->addAction ( m_copyAction.get() );

  // configs button
  conf_buttons();

  // configs tables
  conf_tables();

  // hex editor Tab
  m_layout->addWidget ( m_HexEditor );
  m_ui->HexTab->setLayout ( m_layout );

}

MainWindow::~MainWindow()
{
  delete m_ui;
  delete m_layout;
  delete m_HexEditor;
}

void MainWindow::openFile()
{
  QFileDialog dialog ( this );
  dialog.setFileMode ( QFileDialog::ExistingFile );

  dialog.exec();

  if ( dialog.selectedFiles().count() != 1 )
    return;

  m_FileName = dialog.selectedFiles().at ( 0 );
  parser ( m_FileName );
}

void MainWindow::conf_buttons()
{
#if _WIN32 || _WIN64
  // open
  m_ui->openButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\open.png" ) );
  m_ui->openButton->setShortcut ( QKeySequence ( "Ctrl+O" ) );

  // rpasser
  m_ui->reparseButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\rpasser.png" ) );
  m_ui->reparseButton->setShortcut ( QKeySequence ( "Ctrl+R" ) );

  // hex button
  m_ui->gotoOffsetButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\goto.png" ) );
  m_ui->gotoOffsetButton->setShortcut ( QKeySequence ( "Ctrl+G" ) );

  // full screen
  m_ui->FullScreenButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\show.png" ) );
  m_ui->FullScreenButton->setShortcut ( QKeySequence ( "F11" ) );

  // edit entropy limit
  m_ui->EntroyLimitButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\edit.png" ) );

  // bug button
  m_ui->reportButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\bug.png" ) );

  // new window
  m_ui->newButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\new.png" ) );

  // help button
  m_ui->helpButton->setIcon ( QIcon ( "..\\..\\src\\ui\\assets\\help.png" ) );

#else
  // open
  m_ui->openButton->setIcon ( QIcon ( "../src/ui/assets/open.png" ) );
  m_ui->openButton->setShortcut ( QKeySequence ( "Ctrl+O" ) );

  // rpasser
  m_ui->reparseButton->setIcon ( QIcon ( "../src/ui/assets/rpasser.png" ) );
  m_ui->reparseButton->setShortcut ( QKeySequence ( "Ctrl+R" ) );

  // hex button
  m_ui->gotoOffsetButton->setIcon ( QIcon ( "../src/ui/assets/goto.png" ) );
  m_ui->gotoOffsetButton->setShortcut ( QKeySequence ( "Ctrl+G" ) );

  // full screen
  m_ui->FullScreenButton->setIcon ( QIcon ( "../src/ui/assets/show.png" ) );
  m_ui->FullScreenButton->setShortcut ( QKeySequence ( "F11" ) );

  // edit entropy limit
  m_ui->EntroyLimitButton->setIcon ( QIcon ( "../src/ui/assets/edit.png" ) );

  // bug button
  m_ui->reportButton->setIcon ( QIcon ( "../src/ui/assets/bug.png" ) );

  // new window
  m_ui->newButton->setIcon ( QIcon ( "../src/ui/assets/new.png" ) );

  // help button
  m_ui->helpButton->setIcon ( QIcon ( "../src/ui/assets/help.png" ) );


#endif
}

void MainWindow::conf_tables()
{
  // scoring
  m_ui->scoringTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );

  // symbols
  m_ui->symbolsTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
  
  // header
  m_ui->headerTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );

  // overview
  m_ui->overviewTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );

  // programs
  //m_ui->programsTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );

  // capabilities
  m_ui->capabilitiesTree->horizontalScrollBar();

  // sections
  //m_ui->sectionsTable->horizontalHeader()->setSectionResizeMode ( QHeaderView::Stretch );
}


// TODO: signal error handle
void MainWindow::parser ( QString filename )
{
  // reset
  m_HexEditor->clear();
  m_tableItems.clear();
  m_treeItems.clear();
  m_parser.reset();
  m_ui->overviewTable->clearContents();
  m_ui->headerTable->clearContents();
  m_ui->sectionsTable->clearContents();
  m_ui->programsTable->clearContents();
  m_ui->scoringTable->clearContents();
  m_ui->capabilitiesTree->clear();
  m_ui->scoreDisplay->display ( 0 );
  m_ui->sectionInfo->clear();
  m_ui->programsInfo->clear();
  m_parser.reset ( new ELFParser() );
  m_ui->sizeFile_label->setText("Total 0 Bytes ");
  setWindowTitle ( "elfparser-ng");


  try
  {
    m_parser->parse ( filename.toStdString() );
    m_parser->evaluate();

    m_HexEditor->loadFile ( filename );
  }
  catch ( const std::exception &e )
  {
    std::string errorMessage ( "Loading Error: " );
    errorMessage.append ( e.what() );

    QMessageBox msgBox;
    msgBox.setText ( errorMessage.c_str() );
    msgBox.exec();
    return;
  }

  setWindowTitle ( "elfparser-ng " + QString ( "- " ) + filename );

  // LCD display
  m_ui->scoreDisplay->display ( static_cast<int> ( m_parser->getScore() ) );

  // Overview table
  QTableWidgetItem *tableItem = new QTableWidgetItem ( QString ( m_parser->getFilename().c_str() ) );
  m_ui->overviewTable->setItem ( 0, 0, tableItem );
  m_tableItems.push_back ( tableItem );

  QString sizeFile_str =  QString::number( m_parser->getFileSize() );
  tableItem = new QTableWidgetItem ( sizeFile_str + " Bytes" );
  m_ui->overviewTable->setItem ( 1, 0, tableItem );
  m_ui->sizeFile_label->setText("Total " + sizeFile_str + " Bytes");

  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getMD5().c_str() ) );
  m_ui->overviewTable->setItem ( 2, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getSha1().c_str() ) );
  m_ui->overviewTable->setItem ( 3, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getSha256().c_str() ) );
  m_ui->overviewTable->setItem ( 4, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getFamily().c_str() ) );
  m_ui->overviewTable->setItem ( 5, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  m_VEntropy = m_parser->getEntropy();

  if ( m_VEntropy < m_Entropy )
  {
    tableItem = new QTableWidgetItem (  QString::number ( m_VEntropy ) + " (Not Packed)" );
    m_ui->overviewTable->setItem ( 6, 0, tableItem );
  }
  else
  {
    tableItem = new QTableWidgetItem (  QString::number ( m_VEntropy ) + " (Packed)" );
    m_ui->overviewTable->setItem ( 6, 0, tableItem );
  }

  // elf header view
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getMagic().c_str() ) );
  m_ui->headerTable->setItem ( 0, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getClass().c_str() ) );
  m_ui->headerTable->setItem ( 1, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getEncoding().c_str() ) );
  m_ui->headerTable->setItem ( 2, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getFileVersion().c_str() ) );
  m_ui->headerTable->setItem ( 3, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getOSABI().c_str() ) );
  m_ui->headerTable->setItem ( 4, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getABIVersion().c_str() ) );
  m_ui->headerTable->setItem ( 5, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getType().c_str() ) );
  m_ui->headerTable->setItem ( 6, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getMachine().c_str() ) );
  m_ui->headerTable->setItem ( 7, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  QString versionElf = QString ( m_parser->getElfHeader().getVersion().c_str() );

  if ( versionElf == "1" )
    tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getVersion().c_str() + QString ( " (Current)" ) ) );
  else
    tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getVersion().c_str() ) );

  m_ui->headerTable->setItem ( 8, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getEntryPointString().c_str() ) );
  m_ui->headerTable->setItem ( 9, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getProgramOffset() ).c_str() ) );
  m_ui->headerTable->setItem ( 10, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getSectionOffset() ).c_str() ) );
  m_ui->headerTable->setItem ( 11, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( m_parser->getElfHeader().getFlags().c_str() ) );
  m_ui->headerTable->setItem ( 12, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString::number ( m_parser->getElfHeader().getEHSize() ) );
  m_ui->headerTable->setItem ( 13, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getProgramSize() ).c_str() ) );
  m_ui->headerTable->setItem ( 14, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getProgramCount() ).c_str() ) );
  m_ui->headerTable->setItem ( 15, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getSectionSize() ).c_str() ) );
  m_ui->headerTable->setItem ( 16, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getSectionCount() ).c_str() ) );
  m_ui->headerTable->setItem ( 17, 0, tableItem );
  m_tableItems.push_back ( tableItem );
  tableItem = new QTableWidgetItem ( QString ( boost::lexical_cast<std::string> ( m_parser->getElfHeader().getStringTableIndex() ).c_str() ) );
  m_ui->headerTable->setItem ( 18, 0, tableItem );
  m_tableItems.push_back ( tableItem );

  // sections table
  boost::uint32_t i = 0;
  const std::vector<AbstractSectionHeader> &sections ( m_parser->getSectionHeaders().getSections() );
  m_ui->sectionsTable->setRowCount ( sections.size() );

  BOOST_FOREACH ( const AbstractSectionHeader & section, sections )
  {
    tableItem = new IntWidgetItem ( i );
    m_ui->sectionsTable->setItem ( i, 0, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( section.getName().c_str() ) );
    m_ui->sectionsTable->setItem ( i, 1, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( section.getTypeString().c_str() ) );
    m_ui->sectionsTable->setItem ( i, 2, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( section.getFlagsString().c_str() ) );
    m_ui->sectionsTable->setItem ( i, 3, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( section.getVirtAddress(), true );
    m_ui->sectionsTable->setItem ( i, 4, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( section.getPhysOffset(), false );
    m_ui->sectionsTable->setItem ( i, 5, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( section.getSize() );
    m_ui->sectionsTable->setItem ( i, 6, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( section.getLink() );
    m_ui->sectionsTable->setItem ( i, 7, tableItem );
    m_tableItems.push_back ( tableItem );
    ++i;
  }
  m_ui->sectionsTable->setSortingEnabled ( true );
  m_ui->sectionsTable->resizeRowsToContents();
  m_ui->sectionsTable->resizeColumnsToContents();

  // program headers table
  const std::vector<AbstractProgramHeader> &programs ( m_parser->getProgramHeaders().getProgramHeaders() );
  m_ui->programsTable->setRowCount ( programs.size() );
  m_ui->programsTable->setSortingEnabled ( false );
  i = 0;
  BOOST_FOREACH ( const AbstractProgramHeader & program, programs )
  {
    tableItem = new QTableWidgetItem ( QString ( program.getName().c_str() ) );
    m_ui->programsTable->setItem ( i, 0, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( program.getOffset() );
    m_ui->programsTable->setItem ( i, 1, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( program.getVirtualAddress(), true );
    m_ui->programsTable->setItem ( i, 2, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( program.getPhysicalAddress(), true );
    m_ui->programsTable->setItem ( i, 3, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( program.getFileSize() );
    m_ui->programsTable->setItem ( i, 4, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new IntWidgetItem ( program.getMemorySize() );
    m_ui->programsTable->setItem ( i, 5, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( program.getFlagsString().c_str() ) );
    m_ui->programsTable->setItem ( i, 6, tableItem );
    m_tableItems.push_back ( tableItem );
    ++i;
  }
  m_ui->programsTable->setSortingEnabled ( true );
  m_ui->programsTable->resizeRowsToContents();
  m_ui->programsTable->resizeColumnsToContents();

  // symbols
  const std::vector<AbstractSymbol> &allSymbols ( m_parser->getSegments().getAllSymbols() );
  m_ui->symbolsTable->setRowCount ( allSymbols.size() );
  m_ui->symbolsTable->setSortingEnabled ( false );
  i = 0;
  BOOST_FOREACH ( const AbstractSymbol & symbol, allSymbols )
  {
    tableItem = new QTableWidgetItem ( QString ( symbol.getTypeName().c_str() ) );
    m_ui->symbolsTable->setItem ( i, 0, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( symbol.getBinding().c_str() ) );
    m_ui->symbolsTable->setItem ( i, 1, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( symbol.getName().c_str() ) );
    m_ui->symbolsTable->setItem ( i, 2, tableItem );
    m_tableItems.push_back ( tableItem );
    ++i;
  }
  m_ui->symbolsTable->setSortingEnabled ( true );
  m_ui->symbolsTable->resizeColumnsToContents();

  // capabilities tree
  const std::map<elf::Capabilties, std::set<std::string>> &capabilities ( m_parser->getCapabilties() );

  for ( std::map<elf::Capabilties, std::set<std::string>>::const_iterator it = capabilities.begin();
        it != capabilities.end(); ++it )
  {
    QTreeWidgetItem *rootItem = new QTreeWidgetItem ( m_ui->capabilitiesTree );

    switch ( it->first )
    {
      case elf::k_fileFunctions:
        rootItem->setText ( 0, QString ( "File Functions" ) );
        break;

      case elf::k_networkFunctions:
        rootItem->setText ( 0, QString ( "Network Functions" ) );
        break;

      case elf::k_processManipulation:
        rootItem->setText ( 0, QString ( "Process Manipulation" ) );
        break;

      case elf::k_pipeFunctions:
        rootItem->setText ( 0, QString ( "Pipe Functions" ) );
        break;

      case elf::k_crypto:
        rootItem->setText ( 0, QString ( "Random Functions" ) );
        break;

      case elf::k_infoGathering:
        rootItem->setText ( 0, QString ( "Information Gathering" ) );
        break;

      case elf::k_envVariables:
        rootItem->setText ( 0, QString ( "Environment Variables" ) );
        break;

      case elf::k_permissions:
        rootItem->setText ( 0, QString ( "Permissions" ) );
        break;

      case elf::k_syslog:
        rootItem->setText ( 0, QString ( "System Log" ) );
        break;

      case elf::k_packetSniff:
        rootItem->setText ( 0, QString ( "Packet Sniffing" ) );
        break;

      case elf::k_shell:
        rootItem->setText ( 0, QString ( "Shell" ) );
        break;

      case elf::k_packed:
        rootItem->setText ( 0, QString ( "Packed" ) );
        break;

      case elf::k_irc:
        rootItem->setText ( 0, QString ( "IRC" ) );
        break;

      case elf::k_http:
        rootItem->setText ( 0, QString ( "HTTP" ) );
        break;

      case elf::k_compression:
        rootItem->setText ( 0, QString ( "Compression" ) );
        break;

      case elf::k_ipAddress:
        rootItem->setText ( 0, QString ( "IP Addresses" ) );
        break;

      case elf::k_url:
        rootItem->setText ( 0, QString ( "URL" ) );
        break;

      case elf::k_hooking:
        rootItem->setText ( 0, QString ( "Function Hooking" ) );
        break;

      case elf::k_antidebug:
        rootItem->setText ( 0, QString ( "Anti-Debug" ) );
        break;

      case elf::k_dropper:
        rootItem->setText ( 0, QString ( "Dropper" ) );
        break;

      default:
        rootItem->setText ( 0, QString ( "Unassigned" ) );
        break;
    }

    m_ui->capabilitiesTree->addTopLevelItem ( rootItem );

    BOOST_FOREACH ( const std::string & child, it->second )
    {
      QTreeWidgetItem *childItem = new QTreeWidgetItem ( rootItem );
      childItem->setText ( 1, QString ( child.c_str() ) );
      m_treeItems.push_back ( childItem );
    }
    m_treeItems.push_back ( rootItem );
  }

  m_ui->capabilitiesTree->resizeColumnToContents ( 0 );
  m_ui->capabilitiesTree->resizeColumnToContents ( 1 );

  // score listing
  i = 0;
  const std::vector<std::pair<boost::int32_t, std::string>> &reasons ( m_parser->getReasons() );
  m_ui->scoringTable->setRowCount ( reasons.size() );
  m_ui->scoringTable->setSortingEnabled ( false );

  for ( std::vector<std::pair<boost::int32_t, std::string>>::const_iterator reason = reasons.begin();
        reason != reasons.end(); ++reason )
  {
    tableItem = new IntWidgetItem ( reason->first );
    m_ui->scoringTable->setItem ( i, 0, tableItem );
    m_tableItems.push_back ( tableItem );
    tableItem = new QTableWidgetItem ( QString ( reason->second.c_str() ) );
    m_ui->scoringTable->setItem ( i, 1, tableItem );
    m_tableItems.push_back ( tableItem );
    ++i;
  }

  m_ui->scoringTable->setSortingEnabled ( true );
  m_ui->scoringTable->resizeColumnsToContents();
}

void MainWindow::overviewToClipboard()
{
  QItemSelectionModel *selected = m_ui->overviewTable->selectionModel();

  if ( selected->hasSelection() )
  {
    BOOST_FOREACH ( const QModelIndex & index, selected->selectedRows() )
    {
      QApplication::clipboard()->setText ( m_ui->overviewTable->item ( index.row(), 0 )->text() );
    }
  }
}

void MainWindow::sectionSelected ( QTableWidgetItem *p_first, QTableWidgetItem *p_second )
{
  QTableWidgetItem *selected = m_ui->sectionsTable->item ( p_first->row(), 5 );
  std::string details ( m_parser->getSegments().printSegment ( boost::lexical_cast<boost::uint64_t> ( selected->text().toStdString() ) ) );
  m_ui->sectionInfo->setPlainText ( QString ( details.c_str() ) );
}

void MainWindow::programSelected ( QTableWidgetItem *p_first, QTableWidgetItem *p_second )
{
  QTableWidgetItem *selected = m_ui->programsTable->item ( p_first->row(), 1 );
  std::string details ( m_parser->getSegments().printSegment ( boost::lexical_cast<boost::uint64_t> ( selected->text().toStdString() ) ) );
  m_ui->programsInfo->setPlainText ( QString ( details.c_str() ) );
}

void MainWindow::on_reparseButton_triggered()
{
  if ( m_FileName.size() == 0 )
    return;
  else
    parser ( m_FileName );
}

void MainWindow::on_openButton_triggered()
{
  openFile();
}

void MainWindow::on_gotoOffsetButton_triggered()
{
  if ( m_FileName.size() == 0 )
    return;

  bool done;
  QString offset = QInputDialog::getText ( this, tr ( "Goto..." ),
                   tr ( "Offset (0x for hexadecimal):" ), QLineEdit::Normal,
                   nullptr, &done );

  if ( done && offset[0] == '0' && offset[1] == 'x' )
    m_HexEditor->showFromOffset ( offset.toInt ( nullptr, 16 ) );
  else
    m_HexEditor->showFromOffset ( offset.toInt ( nullptr ) );
}

void MainWindow::on_FullScreenButton_triggered()
{
  if ( isFullScreen() )
    showNormal();
  else
    showFullScreen();
}

void MainWindow::on_EntroyLimitButton_triggered()
{
  bool done;
  double setEntropy = QInputDialog::getDouble ( 0, "Entropy threshold", "Threshold (default 7.0):", m_Entropy, 0, 8, 2, &done );

  if ( done )
    m_Entropy = setEntropy;

}

void MainWindow::on_helpButton_triggered()
{
  QDesktopServices::openUrl ( QUrl ( "https://github.com/mentebinaria/elfparser-ng/wiki/Quick-Help" ) );
}

void MainWindow::on_reportButton_triggered()
{
  QDesktopServices::openUrl ( QUrl ( "https://github.com/mentebinaria/elfparser-ng/issues" ) );
}

void MainWindow::on_newButton_triggered()
{
  auto newWin = new MainWindow();
  newWin->setAttribute ( Qt::WA_DeleteOnClose );
  newWin->show();
}

void MainWindow::on_aboutButton_triggered()
{
  m_dialog.reset ( new QDialog ( this ) );
  Ui_About aboutUi;
  aboutUi.setupUi ( m_dialog.get() );

  m_dialog->exec();
}

void MainWindow::on_headerTable_cellDoubleClicked(int row, int column)
{
  switch(row)
  {
    case 0 :
      m_HexEditor->setSelected(0, 4);
      break;

    case 1:
      m_HexEditor->setSelected(4, 1);
      break;
  }
}

#endif