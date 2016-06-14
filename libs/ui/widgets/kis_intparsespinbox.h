/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISINTPARSESPINBOX_H
#define KISINTPARSESPINBOX_H

#include <QSpinBox>

#include "kritaui_export.h"

/*!
 * \brief The KisDoubleParseSpinBox class is a cleverer doubleSpinBox, able to parse arithmetic expressions.
 *
 * Use this spinbox instead of the basic one from Qt if you want it to be able to parse arithmetic expressions.
 */
class KRITAUI_EXPORT KisIntParseSpinBox : public QSpinBox
{
	Q_OBJECT

public:
		KisIntParseSpinBox(QWidget *parent = 0);
		~KisIntParseSpinBox();

		virtual int valueFromText(const QString & text) const;
		virtual QString textFromValue(int val) const;
		virtual QValidator::State validate ( QString & input, int & pos ) const;

		virtual void stepBy(int steps);

		void setValue(int val); //polymorphism won't work directly, we use a signal/slot hack to do so but if signals are disabled this function will still be usefull.

Q_SIGNALS:

		//! \brief signal emmitted when the last parsed expression create an error.
		void errorWhileParsing(QString expr) const;
		//! \brief signal emmitted when the last parsed expression is valid.
		void noMoreParsingError() const;

public Q_SLOTS:

		//! \brief usefull to let the widget change it's stylesheet when an error occured in the last expression.
		void setErrorStyle();
		//! \brief usefull to let the widget reset it's stylesheet when there's no more error.
		void clearErrorStyle();
		//! \brief say the widget to return to an error free state.
		void clearError();

protected:

		mutable QString* _lastExprParsed;
		mutable bool _isLastValid;
		mutable int _oldVal; //store the last correctly evaluated value, to
};

#endif // KISINTPARSESPINBOX_H
