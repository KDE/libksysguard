/*
    KSysGuard, the KDE System Guard

        Copyright (C) 2007 Trent Waddington <trent.waddington@gmail.com>
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

#include <klocale.h>
#include "KTextEditVT.h"

#include "KTextEditVT.moc"

KTextEditVT::KTextEditVT(QWidget* parent)
	: QTextEdit( parent )
{
	mParseAnsi = true;
	escape_sequence = false;
	escape_bracket = false;
	escape_number = -1;
	escape_code = 0;
}


void KTextEditVT::insertVTChar(const QChar & c) {
	if(c.isPrint() || c == '\n') { 
		if(escape_sequence) {
			if(escape_bracket) {
				if(c.isDigit()) {
					if(escape_number == -1)
						escape_number = c.digitValue();
					else 
						escape_number = escape_number*10 + c.digitValue();
				} else {
					escape_code = c;
				}
			
			} else if(c=='[') {
				escape_bracket = true;
			}
			else if(c=='(' || c==')') {}
			else
				escape_code = c;
			if(!escape_code.isNull()) {
				//We've read in the whole escape sequence.  Now parse it
				if(escape_code == 'm') { // change color
					switch(escape_number){
						case 0: //all off
							setFontWeight(QFont::Normal);
							setTextColor(Qt::black);
							break;
						case 1: //bold
							setFontWeight(QFont::Bold);
							break;
						case 31: //red
							setTextColor(Qt::red);
							break;
						case 32: //green
							setTextColor(Qt::green);
							break;
						case 33: //yellow
							setTextColor(Qt::yellow);
							break;
						case 34: //blue
							setTextColor(Qt::blue);
							break;
						case 35: //magenta
							setTextColor(Qt::magenta);
							break;
						case 36: //cyan
							setTextColor(Qt::cyan);
							break;
						case -1:
						case 30: //black
						case 39: //reset
						case 37: //white
							setTextColor(Qt::black);
							break;
					}
				}
				escape_code = 0;
				escape_number = -1;
				escape_bracket = false;
				escape_sequence = false;
			}
		} else
			insertPlainText(QChar(c));

	}
	else if(c == 0x0d)
		insertPlainText(QChar('\n'));
	else if(mParseAnsi) {
		if(c == 127 || c == 8) { // delete or backspace, respectively
			textCursor().deletePreviousChar();
		} else if(c==27) { // escape key
			escape_sequence = true;
		} else if(c==0x9b) { // CSI - equivalent to esc [
			escape_sequence = true;
			escape_bracket = true;
		}

	}
	else if(!c.isNull()) {
		insertPlainText("[");
		QByteArray num;
		num.setNum(c.toAscii());
		insertPlainText(num);
		insertPlainText("]");
	}
}

void KTextEditVT::insertVTText(const QByteArray & string)
{
	int size= string.size();
	for(int i =0; i < size; i++)
		insertVTChar(QChar(string.at(i)));
}

void KTextEditVT::insertVTText(const QString & string)
{
	int size= string.size();
	for(int i =0; i < size; i++)
		insertVTChar(string.at(i));
}

void KTextEditVT::setParseAnsiEscapeCodes(bool parseAnsi)
{
	mParseAnsi = parseAnsi;
}

bool KTextEditVT::parseAnsiEscapeCodes()
{
	return mParseAnsi;
}

