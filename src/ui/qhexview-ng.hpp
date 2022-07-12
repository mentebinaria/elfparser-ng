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

// config font
#define FONT "Consolas"
#define SIZE_FONT 11
#define COLOR_SELECTION 98, 114, 164, 0xff

// config lines
#define MIN_HEXCHARS_IN_LINE 47
#define GAP_ADR_HEX 10
#define GAP_HEX_ASCII 16
#define MIN_BYTES_PER_LINE 16
#define ADR_LENGTH 10

class QHexView : public QAbstractScrollArea
{
  Q_OBJECT
public:
  QHexView(QWidget *parent = nullptr);
  ~QHexView();

protected:
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);

private:
  QByteArray m_pdata;

  unsigned int m_posAddr,
      m_posHex,
      m_posAscii,
      m_charWidth,
      m_charHeight,
      m_selectBegin,
      m_selectEnd,
      m_selectInit,
      m_cursorPos,
      m_bytesPerLine;

  QColor m_colorCharacter;
  QColor m_colorAddress;

  QSize fullSize() const;
  void updatePositions();
  void resetSelection();
  void resetSelection(int pos);
  void setSelection(int pos);
  void ensureVisible();
  void setCursorPos(int pos);
  int cursorPos(const QPoint &position);
  void paintMark(int xpos, int ypos);
  void confScrollBar();
  void copyBytes();

public slots:
  void loadFile(QString p_file);
  void clear();
  void showFromOffset(int offset);
  void setSelected(int offset, int length);
  unsigned int getOffset();
  void setColorCharacters(const QColor &color);
  void setColorAddress(const QColor &color);
};

#endif
