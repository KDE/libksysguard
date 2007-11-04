/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2006 John Tapsell <john.tapsell@kdemail.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <QtCore/QAbstractItemModel>

#include <kdemacros.h>

namespace KSysGuard {
	class Processes;
	class Process;
}

class ProcessModelPrivate;
class KDE_EXPORT ProcessModel : public QAbstractItemModel
{
	Q_OBJECT
	Q_ENUMS(Units)
		
public:
	ProcessModel(QObject* parent = 0);
	virtual ~ProcessModel();

	/* Functions for our Model for QAbstractItemModel*/
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	QModelIndex parent ( const QModelIndex & index ) const;
	
	bool hasChildren ( const QModelIndex & parent) const;
	
	/* Functions for setting the model */

	/** Setup the column headings by inserting the appropriate headings into the model.
	 *  Can be called more than once to retranslate the headings if the system language changes.
	 */
	void setupHeader();

	/** Update data.  You can pass in the time between updates to only update if there hasn't
	 *  been an update within the last @p updateDurationMSecs milliseconds */
	void update(int updateDurationMSecs = 0);
	
	/** Return a string with the pid of the process and the name of the process.  E.g.  13343: ksyguard
	 */
	QString getStringForProcess(KSysGuard::Process *process) const;
	KSysGuard::Process *getProcess(long long pid);

	/** This is used from ProcessFilter to get the process at a given index when in flat mode */	
	KSysGuard::Process *getProcessAtIndex(int index) const;
        
	/** Returns whether this user can log in or not.
	 *  @see mUidCanLogin
	 */
	bool canUserLogin(long long uid) const;
	/** In simple mode, everything is flat, with no icons, few if any colors, no xres etc.
	 *  This can be changed at any time.  It is a fairly quick operation.  Basically it resets the model
	 */ 
	void setSimpleMode(bool simple);
	/** In simple mode, everything is flat, with no icons, few if any colors, no xres etc
	 */
	bool isSimpleMode() const;
	
	/** Returns the total amount of physical memory in the machine. */
	long long totalMemory() const;

        /** This returns a QModelIndex for the given process.  It has to look up the parent for this pid, find the offset this 
	 *  pid is from the parent, and return that.  It's not that slow, but does involve a couple of hash table lookups.
	 */
	QModelIndex getQModelIndex ( KSysGuard::Process *process, int column) const;

	/** Whether this is showing the processes for the current machine
	 */
	bool isLocalhost() const;

	/** Returns for process controller pointer for this model
	 */
	KSysGuard::Processes *processController();   ///The processes instance

	/** The headings in the model.  The order here is the order that they are shown
	 *  in.  If you change this, make sure you also change the 
	 *  setup header function
	 */
	enum { HeadingName=0, HeadingUser, HeadingTty, HeadingNiceness, HeadingCPUUsage, HeadingVmSize, HeadingMemory, HeadingSharedMemory, HeadingCommand, HeadingXTitle };

	bool showTotals() const;

	/** When displaying memory sizes, this is the units it should be displayed in */
	enum Units { UnitsKB, UnitsMB, UnitsGB  };
	/** Set the units memory sizes etc should be displayed in */
	void setUnits(Units units);
	/** The units memory sizes etc should be displayed in */
	Units units() const;
	/** Take an amount in kb, and return a string in the units set by setUnits() */
	QString formatMemoryInfo(long amountInKB) const;

	/** Retranslate the GUI, for when the system language changes */
	void retranslateUi();

public Q_SLOTS:

	/** Whether to show the total cpu for the process plus all of its children */
	void setShowTotals(bool showTotals);

private:
	ProcessModelPrivate*  const d;
	friend class ProcessModelPrivate;
};

#endif

