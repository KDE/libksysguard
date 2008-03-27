/*
    KSysGuard, the KDE System Guard

    Copyright (c) 2008 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef _KTextEditVT_h_
#define _KTextEditVT_h_

#include <QTextEdit>

class KTextEditVT : public QTextEdit
{
	Q_OBJECT

public:
	KTextEditVT(QWidget* parent);
public Q_SLOTS:
	/** Insert the given character at the current position based on the current state.
	 *  This is interpreted in a VT100 encoding.  Backspace and delete will delete the previous character,
	 *  escape sequences can move the cursor and set the current color etc.
	 */
	void insertVTChar(const QChar & c);

private:
	bool eight_bit_clean;
	bool escape_sequence;
	bool escape_bracket;
	int escape_number;
	QChar escape_code;

};

#endif
