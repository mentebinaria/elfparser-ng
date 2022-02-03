/**
 * @file hex_editor.hpp
 * @author Mente Binaria
 * @brief hex editor viewing all loaded elf binary
 * @version 1.5.0
 * @date 2022-01-25
 *
 * @copyright Copyright (c) 2022
 *
 * @note This library is a fork from virinext`s QHexView
 *       https://github.com/virinext/QHexView
 */

#pragma once

#include <QByteArray>
#include <QAbstractScrollArea>
#include <QScrollBar>
#include "src/structures/error.hpp"

class HexEditor : public QAbstractScrollArea
{
    Q_OBJECT
    
private:
    QByteArray m_BufferHex;

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

    void confScrollBar();
    void updatePositions();
    void resetSelection();
    void resetSelection(std::size_t pos);
    void setSelection(int pos);
    void ensureVisible();
    void setCursorPos(int pos);

    QSize fullSize() const;
    std::size_t cursorPos(const QPoint &pos);

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

public:
    explicit HexEditor(QWidget *parent = nullptr);
    ~HexEditor();
    int  LoadBinary(const QString &__fpath);
    int  CallDialog();
    void Clear();
};