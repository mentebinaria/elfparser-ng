#ifdef QT_GUI

#include "qhexview-ng.hpp"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSize>
#include <stdexcept>
#include <QtGlobal>

#include <iostream>

// update
#define UPDATE viewport()->update();

// valid character table ascii
#define CHAR_VALID(caracter) \
  ((caracter < 0x20) || (caracter > 0x7e)) ? caracter = '.' : caracter;

// mark selection
#define SET_BACKGROUND_MARK(pos)                            \
  if (pos >= m_selectBegin && pos < m_selectEnd)            \
  {                                                         \
    painter.setBackground(QBrush(QColor(COLOR_SELECTION))); \
    painter.setBackgroundMode(Qt::OpaqueMode);              \
  }

QHexView::QHexView(QWidget *parent)
    : QAbstractScrollArea(parent),
      m_pdata(nullptr),
      m_posAddr(0),
      m_charWidth(0),
      m_posHex(ADR_LENGTH * m_charWidth + GAP_ADR_HEX),
      m_posAscii(m_posHex + MIN_HEXCHARS_IN_LINE * m_charWidth + GAP_HEX_ASCII),
      m_bytesPerLine(MIN_BYTES_PER_LINE),
      m_charHeight(0),
      m_selectBegin(0),
      m_selectEnd(0),
      m_selectInit(0),
      m_cursorPos(0)
{
  // default configs
  setFont(QFont(FONT, SIZE_FONT)); // default font
}

QHexView::~QHexView() {}

void QHexView::loadFile(QString p_file)
{
  QFile qFile;

  qFile.setFileName(p_file);

  qFile.open(QFile::ReadOnly);

  if (qFile.isOpen())
  {
    setCursorPos(0);
    resetSelection(0);

    m_pdata = qFile.readAll();

    qFile.close();
  }
  else
    throw std::runtime_error("Falied to open file " + p_file.toStdString() + " not possible len bin");

  resetSelection(0);
}

// search and set offset
void QHexView::showFromOffset(int offset)
{
  if (offset <= m_pdata.size())
  {
    updatePositions();

    setCursorPos(offset * 2);
    setSelected(offset, 1);

    int cursorY = m_cursorPos / (2 * m_bytesPerLine);

    verticalScrollBar()->setValue(cursorY);
    UPDATE
  }
  else
    throw std::runtime_error("Offset invalid, verify offset " + std::to_string(offset));
}

// clean all
void QHexView::clear()
{
  verticalScrollBar()->setValue(0);
  m_pdata.clear();
  UPDATE;
}

QSize QHexView::fullSize() const
{
  if (m_pdata.size() == 0)
    return QSize(0, 0);

  int width = m_posAscii + (m_bytesPerLine * m_charWidth);
  int height = m_pdata.size() / m_bytesPerLine;

  if (m_pdata.size() % m_bytesPerLine)
    height++;

  height *= m_charHeight;

  return QSize(width, height);
}

void QHexView::updatePositions()
{
#if QT_VERSION >= 0x50f03
  m_charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
#else
  m_charWidth = fontMetrics().width(QLatin1Char('9'));
#endif

  m_charHeight = fontMetrics().height();

  int serviceSymbolsWidth = ADR_LENGTH * m_charWidth + GAP_ADR_HEX + GAP_HEX_ASCII;

  m_bytesPerLine = (width() - serviceSymbolsWidth) / (4 * m_charWidth) - 1; // 4 symbols per byte

  if (m_bytesPerLine < 5) // avoid floating point
    m_bytesPerLine = 5;

  m_posAddr = 0;
  m_posHex = ADR_LENGTH * m_charWidth + GAP_ADR_HEX;
  m_posAscii = m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth + GAP_HEX_ASCII;
}

/*****************************************************************************/
/* Paint Hex View  */
/*****************************************************************************/
void QHexView::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  updatePositions();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + viewport()->size().height() / m_charHeight;

  int linePos = m_posAscii - (GAP_HEX_ASCII / 2);
  int yPosStart = m_charHeight;
  QBrush def = painter.brush();

  // paint background address
  painter.fillRect(event->rect(), this->palette().color(QPalette::Base));
  painter.fillRect(QRect(m_posAddr, event->rect().top(), m_posHex - GAP_ADR_HEX + 10, height()), m_colorAddress);
  painter.setBackgroundMode(Qt::OpaqueMode);

  // paint line separate ascii
  painter.setPen(m_colorAddress);
  painter.drawLine(linePos, event->rect().top(), linePos, height());

  const QByteArray data = m_pdata.mid(firstLineIdx * m_bytesPerLine, (lastLineIdx - firstLineIdx) * m_bytesPerLine);

  painter.setPen(m_colorCharacter); // paint white characters and binary

  if (m_pdata.size() == 0)
    return;

  confScrollBar();

  for (int lineIdx = firstLineIdx, yPos = yPosStart; lineIdx < lastLineIdx;
       lineIdx += 1, yPos += m_charHeight)
  {
    // ascii position
    for (int xPosAscii = m_posAscii, i = 0;
         ((lineIdx - firstLineIdx) * m_bytesPerLine + i) < data.size() &&
         (i < m_bytesPerLine);
         i++, xPosAscii += m_charWidth)
    {
      SET_BACKGROUND_MARK((lineIdx * m_bytesPerLine + i) * 2);

      char character = data[(lineIdx - firstLineIdx) * (uint)m_bytesPerLine + i];
      CHAR_VALID(character);

      painter.drawText(xPosAscii, yPos, QString(character));

      painter.setBackground(painter.brush());
      painter.setBackgroundMode(Qt::OpaqueMode);
    }

    // binary position
    for (int xPos = m_posHex, i = 0; i < m_bytesPerLine &&
                                     ((lineIdx - firstLineIdx) * m_bytesPerLine + i) < data.size();
         i++, xPos += 3 * m_charWidth)
    {
      SET_BACKGROUND_MARK((lineIdx * m_bytesPerLine + i) * 2);

      QString val = QString::number((data.at((lineIdx - firstLineIdx) * m_bytesPerLine + i) & 0xF0) >> 4, 16);
      painter.drawText(xPos, yPos, val);

      val = QString::number((data.at((lineIdx - firstLineIdx) * m_bytesPerLine + i) & 0xF), 16);
      painter.drawText(xPos + m_charWidth, yPos, val);

      painter.setBackground(painter.brush());
      painter.setBackgroundMode(Qt::OpaqueMode);
    }

    // offsets
    QString address = QString("%1:").arg(lineIdx * m_bytesPerLine, 10, 16, QChar('0'));
    painter.drawText(m_posAddr, yPos, address);

    // cursor drawn
    if (hasFocus())
    {
      int x = (m_cursorPos % (2 * m_bytesPerLine));
      int y = m_cursorPos / (2 * m_bytesPerLine);
      y -= firstLineIdx;
      int cursorX = (((x / 2) * 3) + (x % 2)) * m_charWidth + m_posHex;
      int cursorY = y * m_charHeight + 4;
      painter.fillRect(cursorX, cursorY, 2, m_charHeight, palette().color(QPalette::WindowText));
    }
  }
}

void QHexView::keyPressEvent(QKeyEvent *event)
{
  bool setVisible = false;

  /*****************************************************************************/
  /* Cursor movements */
  /*****************************************************************************/
  if (event->matches(QKeySequence::MoveToNextChar))
  {
    setCursorPos(m_cursorPos + 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToPreviousChar))
  {
    setCursorPos(m_cursorPos - 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToEndOfLine))
  {
    setCursorPos(m_cursorPos | ((m_bytesPerLine * 2) - 2));
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToStartOfLine))
  {
    setCursorPos(m_cursorPos | (m_cursorPos % (m_bytesPerLine * 2)));
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToPreviousLine))
  {
    setCursorPos(m_cursorPos - m_bytesPerLine * 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToNextLine))
  {
    setCursorPos(m_cursorPos + m_bytesPerLine * 2);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToNextPage))
  {
    setCursorPos(m_cursorPos + (viewport()->height() / m_charHeight - 2) *
                                   2 * m_bytesPerLine);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToPreviousPage))
  {
    setCursorPos(m_cursorPos - (viewport()->height() / m_charHeight - 2) *
                                   2 * m_bytesPerLine);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToEndOfDocument))
  {
    if (m_pdata.size())
      setCursorPos(m_pdata.size() * 2);

    resetSelection(m_cursorPos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::MoveToStartOfDocument))
  {
    setCursorPos(0);
    resetSelection(m_cursorPos);
    setVisible = true;
  }

  /*****************************************************************************/
  /* Select commands */
  /*****************************************************************************/
  if (event->matches(QKeySequence::SelectAll))
  {
    resetSelection(0);

    if (m_pdata.size())
      setSelection(2 * m_pdata.size() + 2);

    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectNextChar))
  {
    int pos = m_cursorPos + 2;
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectPreviousChar))
  {
    int pos = m_cursorPos - 2;
    setSelection(pos);
    setCursorPos(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectEndOfLine))
  {
    int pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine)) +
              (2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectStartOfLine))
  {
    int pos = m_cursorPos - (m_cursorPos % (2 * m_bytesPerLine));
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectPreviousLine))
  {
    int pos = m_cursorPos - (2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectNextLine))
  {
    int pos = m_cursorPos + (2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectNextPage))
  {
    int pos = m_cursorPos + (((viewport()->height() / m_charHeight) - 2) *
                             2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectPreviousPage))
  {
    int pos = m_cursorPos - (((viewport()->height() / m_charHeight) - 2) *
                             2 * m_bytesPerLine);
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectEndOfDocument))
  {
    int pos = 0;

    if (m_pdata.size())
      pos = m_pdata.size() * 2;

    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::SelectStartOfDocument))
  {
    int pos = 0;
    setCursorPos(pos);
    setSelection(pos);
    setVisible = true;
  }

  if (event->matches(QKeySequence::Copy))
    copyBytes();

  if (setVisible)
    ensureVisible();

  UPDATE;
}

void QHexView::mouseMoveEvent(QMouseEvent *event)
{
  int actPos = cursorPos(event->pos());

  if (actPos != std::numeric_limits<int>::max())
  {
    setCursorPos(actPos);
    setSelection(actPos);
  }

  UPDATE;
}

void QHexView::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::RightButton)
  {
    event->ignore();
    return;
  }

  int cPos = cursorPos(event->pos());

  if ((QApplication::keyboardModifiers() & Qt::ShiftModifier))
    setSelection(cPos);
  else
    resetSelection(cPos);

  if (cPos != std::numeric_limits<int>::max())
    setCursorPos(cPos);

  UPDATE;
}

int QHexView::cursorPos(const QPoint &position)
{
  int pos = std::numeric_limits<int>::max();

  if (((int)position.x() >= m_posHex) &&
      ((int)position.x() <
       (m_posHex + (m_bytesPerLine * 3 - 1) * m_charWidth)))
  {
    int x = (position.x() - m_posHex) / m_charWidth;

    ((x % 3) == 0) ? x = (x / 3) * 2 : x = ((x / 3) * 2) + 2;

    int firstLineIdx = verticalScrollBar()->value();
    int y = (position.y() / m_charHeight) * 2 * m_bytesPerLine;
    pos = x + y + firstLineIdx * m_bytesPerLine * 2;
  }

  return pos;
}

void QHexView::resetSelection()
{
  m_selectBegin = m_selectInit;
  m_selectEnd = m_selectInit;
}

void QHexView::resetSelection(int pos)
{
  if (pos == std::numeric_limits<int>::max())
    pos = 0;

  m_selectInit = pos;
  m_selectBegin = pos;
  m_selectEnd = pos;
}

void QHexView::setSelection(int pos)
{
  if (pos == std::numeric_limits<int>::max())
    pos = 0;

  if (pos >= m_selectInit)
  {
    m_selectEnd = pos;
    m_selectBegin = m_selectInit;
  }
  else
  {
    m_selectBegin = pos;
    m_selectEnd = m_selectInit;
  }
}

void QHexView::setSelected(int offset, int length)
{
  m_selectInit = m_selectBegin = offset * 2;
  m_selectEnd = m_selectBegin + length * 2;

  setCursorPos(offset * 2);
  UPDATE
}

void QHexView::setCursorPos(int position)
{
  if (position == std::numeric_limits<int>::max())
    position = 0;

  int maxPos = 0;

  if (m_pdata.size() != 0)
  {
    maxPos = m_pdata.size() * 2;

    if (m_pdata.size() % m_bytesPerLine)
      maxPos += 1;
  }

  if (position > maxPos)
    position = maxPos;

  m_cursorPos = position;
}

unsigned int QHexView::getOffset()
{
  return m_cursorPos / 2;
}

void QHexView::ensureVisible()
{
  QSize areaSize = viewport()->size();

  int firstLineIdx = verticalScrollBar()->value();
  int lastLineIdx = firstLineIdx + areaSize.height() / m_charHeight;

  int cursorY = m_cursorPos / (2 * m_bytesPerLine);

  if (cursorY < firstLineIdx)
    verticalScrollBar()->setValue(cursorY);
  else if (cursorY >= lastLineIdx)
    verticalScrollBar()->setValue(cursorY - areaSize.height() / m_charHeight + 1);
}

void QHexView::confScrollBar()
{
  QSize areaSize = viewport()->size();
  QSize widgetSize = fullSize();
  verticalScrollBar()->setPageStep(areaSize.height() / m_charHeight);
  verticalScrollBar()->setRange(0, (widgetSize.height() - areaSize.height()) / m_charHeight + 1);
}

void QHexView::copyBytes()
{
  if (m_pdata.size())
  {
    QString res;
    int idx = 0;
    int copyOffset = 0;

    QByteArray data = m_pdata.mid(m_selectBegin / 2,
                                  (m_selectEnd - m_selectBegin) / 2 + 2);

    if (m_selectBegin % 2)
    {
      res += QString::number((data.at((idx + 2) / 2) & 0xF), 16);
      res += " ";
      idx++;
      copyOffset = 1;
    }

    int selectedSize = m_selectEnd - m_selectBegin;

    for (; idx < selectedSize; idx += 2)
    {
      if (data.size() > (copyOffset + idx) / 2)
      {
        QString val = QString::number(
            (data.at((copyOffset + idx) / 2) & 0xF0) >> 4, 16);

        if (idx + 2 < selectedSize)
        {
          val += QString::number(
              (data.at((copyOffset + idx) / 2) & 0xF), 16);
          val += " ";
        }

        res += val;

        if ((idx / 2) % m_bytesPerLine == (m_bytesPerLine - 1))
          res += "\n";
      }
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(res);
  }
}

void QHexView::setColorCharacters(const QColor &color)
{
  m_colorCharacter = color;
}

void QHexView::setColorAddress(const QColor &color)
{
  m_colorAddress = color;
}

#endif