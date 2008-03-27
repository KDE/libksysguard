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

/*
 *    \class KTextEditVT
 *   \brief The KTextEditVT class provides a widget that is used to edit and display
 *   both plain and rich text with the additional function of being able to 
 *   programmatically append VT100 formatted text.  For example to display the output
 *   from console programs.
 *
 *    \mainclass
 *
 *    \tableofcontents
 *
 *    \section1 Basics
 *    This class can be used to display the output of VT100 formatted text with
 *    ANSI escape code - for example output from the command 'ls --color'.
 *
 *    Only a very limited number of ansi escapes sequences will have an affect.  Unrecognised
 *    ansi escape sequences will be ignored and not displayed.  Patches are welcome to support
 *    more of the sequences.
 *
 *    This output can be then be inserted at the current cursor position by calling
 *    insertVTText(string);
 *
 *    For example:
 *
 *    \code
 *      insertVTText(QString("Hi") + QChar(08) + "ello");
 *    \endcode
 *    
 *    will insert the text  "Hello" at the current character position.  
 *    (Character 08 is the literal backspace character.  Treated as equivalent to character 127)
 */
class KDE_EXPORT KTextEditVT : public QTextEdit
{
	Q_OBJECT

public:
	KTextEditVT(QWidget* parent);
public Q_SLOTS:
	/** Insert the given string at the current position based on the current state.
	 *  This is interpreted in a VT100 encoding.  Backspace and delete will delete the previous character,
	 *  escape sequences can move the cursor and set the current color etc.
	 *  
	 *  This just calls insertVTChar for each character in the string
	 */
	void insertVTText(const QByteArray & string);
	/** Insert the given string at the current position based on the current state.
	 *  This is interpreted in a VT100 encoding.  Backspace and delete will delete the previous character,
	 *  escape sequences can move the cursor and set the current color etc.
	 *
	 *  This just calls insertVTChar for each character in the string
	 */
	void insertVTText(const QString & string);

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
