/**
* @file QHexView.cpp
* @author VitorMob 2022
* @date 15 Mar 2022
* @copyright 2022 VitorMob, 2015 virinext
* @brief simple hex view / editor
*/

#ifndef Q_HEX_VIEWER_H_
#define Q_HEX_VIEWER_H_

#include <QAbstractScrollArea>
#include <QByteArray>
#include <QFile>
#include <QMutex>

class QHexView: public QAbstractScrollArea

{
	Q_OBJECT
	public:
		class DataStorage
		{
			public:
				virtual ~DataStorage() {};
				virtual QByteArray getData(int position, int length) = 0;
				virtual int size() = 0;
		};


		class DataStorageArray: public DataStorage
		{
			public:
				DataStorageArray(const QByteArray &arr);
				virtual QByteArray getData(int position, int length);
				virtual int size();
			private:
				QByteArray    m_data;
		};

		class DataStorageFile: public DataStorage
		{
			public:
				DataStorageFile(const QString &fileName);
				virtual QByteArray getData(int position, int length);
				virtual int size();
			private:
				QFile      m_file;
		};



		QHexView(QWidget *parent = 0);
		~QHexView();

	public slots:
		void setData(DataStorage *pData);
		void clear();
		void showFromOffset(int offset);
		void setSelected(int offset, int length);

	protected:
		void paintEvent(QPaintEvent *event);
		void keyPressEvent(QKeyEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
	private:
		QMutex        m_dataMtx;
		DataStorage  *m_pdata;
		int           m_posAddr;
		int           m_posHex;
		int           m_posAscii;
		int           m_charWidth;
		int           m_charHeight;

		int           m_selectBegin;
		int           m_selectEnd;
		int           m_selectInit;
		int           m_cursorPos;
		int           m_bytesPerLine;

		QSize fullSize() const;
		void updatePositions();
		void resetSelection();
		void resetSelection(int pos);
		void setSelection(int pos);
		void ensureVisible();
		void setCursorPos(int pos);
		int  cursorPos(const QPoint &position);
};

#endif
