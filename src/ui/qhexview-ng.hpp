/**
* @file QHexView.cpp
* @author VitorMob 2022
* @date 15 Mar 2022
* @copyright 2022 VitorMob, 2015 virinext
* @brief simple hex view / editor
*/

#ifdef QT_GUI

#pragma once

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>


// config colors
#if _WIN32 || _WIN64
  // config font
  #define FONT "Courier"
  #define SIZE_FONT 10
  #define COLOR_CHARACTERS Qt::black
  #define COLOR_SELECTION 98, 114, 164, 0xff
  #define COLOR_ADDRESS 240, 240, 240, 0xff
#else
  // config font
  #define FONT "Courier"
  #define SIZE_FONT 12
  #define COLOR_SELECTION 98, 114, 164, 0xff
  #define COLOR_ADDRESS 30, 30, 30, 0xff
  #define COLOR_CHARACTERS Qt::white
#endif

// config lines
#define MIN_HEXCHARS_IN_LINE 47
#define GAP_ADR_HEX 10
#define GAP_HEX_ASCII 16
#define MIN_BYTES_PER_LINE 16
#define ADR_LENGTH 10


class QHexView: public QAbstractScrollArea

{
  Q_OBJECT
 public:
  QHexView ( QWidget *parent = nullptr );
  ~QHexView();


 protected:
  void paintEvent ( QPaintEvent *event );
  void keyPressEvent ( QKeyEvent *event );
  void mouseMoveEvent ( QMouseEvent *event );
  void mousePressEvent ( QMouseEvent *event );

 private:
  QByteArray  m_pdata;

  int m_posAddr,
      m_posHex,
      m_posAscii,
      m_charWidth,
      m_charHeight,
      m_selectBegin,
      m_selectEnd,
      m_selectInit,
      m_cursorPos,
      m_bytesPerLine;

  QSize fullSize() const;
  void updatePositions();
  void resetSelection();
  void resetSelection ( int pos );
  void setSelection ( int pos );
  void ensureVisible();
  void setCursorPos ( int pos );
  int  cursorPos ( const QPoint &position );
  int  getCursorPos();
  void paintMark(int xpos, int ypos);
  void confScrollBar();

 public slots:
  void loadFile ( QString p_file );
  void clear();
  void showFromOffset ( int offset );
  void setSelected ( int offset, int length );
};

#endif