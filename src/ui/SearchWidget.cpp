/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2014 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "SearchWidget.h"
#include "PreferencesDialog.h"
#include "SearchDelegate.h"
#include "../core/SearchesManager.h"
#include "../core/SearchSuggester.h"
#include "../core/SessionsManager.h"
#include "../core/SettingsManager.h"

#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QLineEdit>

namespace Otter
{

SearchWidget::SearchWidget(QWidget *parent) : QComboBox(parent),
	m_completer(new QCompleter(this)),
	m_suggester(NULL),
	m_index(0),
	m_sendRequest(false)
{
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setCompletionMode(QCompleter::PopupCompletion);
	m_completer->setCompletionRole(Qt::DisplayRole);

	setEditable(true);
	setInsertPolicy(QComboBox::NoInsert);
	setItemDelegate(new SearchDelegate(this));
	setModel(SearchesManager::getSearchEnginesModel());
	setCurrentSearchEngine();
	optionChanged(QLatin1String("Browser/SearchEnginesSuggestions"), SettingsManager::getValue(QLatin1String("Browser/SearchEnginesSuggestions")));
	lineEdit()->setCompleter(m_completer);

	connect(SearchesManager::getInstance(), SIGNAL(searchEnginesModified()), this, SLOT(setCurrentSearchEngine()));
	connect(SettingsManager::getInstance(), SIGNAL(valueChanged(QString,QVariant)), this, SLOT(optionChanged(QString,QVariant)));
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(currentSearchEngineChanged(int)));
	connect(this, SIGNAL(activated(int)), this, SLOT(searchEngineSelected(int)));
	connect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(queryChanged(QString)));
	connect(lineEdit(), SIGNAL(returnPressed()), this, SLOT(sendRequest()));
	connect(m_completer, SIGNAL(activated(QString)), this, SLOT(sendRequest(QString)));
}

void SearchWidget::wheelEvent(QWheelEvent *event)
{
	disconnect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(queryChanged(QString)));

	QComboBox::wheelEvent(event);

	connect(lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(queryChanged(QString)));
}

void SearchWidget::hidePopup()
{
	if (!m_query.isEmpty())
	{
		m_sendRequest = true;
	}

	QComboBox::hidePopup();
}

void SearchWidget::optionChanged(const QString &option, const QVariant &value)
{
	if (option == QLatin1String("Browser/SearchEnginesSuggestions"))
	{
		if (value.toBool() && !m_suggester)
		{
			m_suggester = new SearchSuggester(getCurrentSearchEngine(), this);

			m_completer->setModel(m_suggester->getModel());

			connect(lineEdit(), SIGNAL(textEdited(QString)), m_suggester, SLOT(setQuery(QString)));
		}
		else if (!value.toBool() && m_suggester)
		{
			m_suggester->deleteLater();
			m_suggester = NULL;

			m_completer->setModel(NULL);
		}
	}
}

void SearchWidget::currentSearchEngineChanged(int index)
{
	if (itemData(index, Qt::AccessibleDescriptionRole).toString().isEmpty())
	{
		lineEdit()->setPlaceholderText(tr("Search Using %1").arg(itemData(index, Qt::UserRole).toString()));
	}
	else
	{
		lineEdit()->setPlaceholderText(QString());
	}

	lineEdit()->setText(m_query);
}

void SearchWidget::searchEngineSelected(int index)
{
	if (itemData(index, Qt::AccessibleDescriptionRole).toString().isEmpty())
	{
		m_index = index;

		if (m_suggester)
		{
			m_suggester->setEngine(getCurrentSearchEngine());
			m_suggester->setQuery(QString());
		}

		SessionsManager::markSessionModified();
	}
	else
	{
		setCurrentIndex(m_index);

		if (itemData(index, Qt::AccessibleDescriptionRole).toString() == QLatin1String("configure"))
		{
			PreferencesDialog dialog(QLatin1String("search"), this);
			dialog.exec();
		}
	}
}

void SearchWidget::queryChanged(const QString &query)
{
	if (m_sendRequest)
	{
		sendRequest();

		m_sendRequest = false;
	}

	m_query = query;
}

void SearchWidget::sendRequest(const QString &query)
{
	if (!query.isEmpty())
	{
		m_query = query;
	}

	if (!m_query.isEmpty())
	{
		emit requestedSearch(m_query, itemData(currentIndex(), (Qt::UserRole + 1)).toString());
	}

	m_query = QString();

	lineEdit()->clear();
}

void SearchWidget::setCurrentSearchEngine(const QString &engine)
{
	const QStringList engines = SearchesManager::getSearchEngines();

	if (engines.isEmpty())
	{
		hidePopup();
		setEnabled(false);

		lineEdit()->setPlaceholderText(QString());

		return;
	}

	setEnabled(true);

	if (sender() == SearchesManager::getInstance() && engines.contains(getCurrentSearchEngine()))
	{
		currentSearchEngineChanged(currentIndex());

		return;
	}

	const int index = qMax(0, engines.indexOf(engine.isEmpty() ? SettingsManager::getValue(QLatin1String("Browser/DefaultSearchEngine")).toString() : engine));

	setCurrentIndex(index);
	currentSearchEngineChanged(index);
	searchEngineSelected(index);

	if (m_suggester)
	{
		m_suggester->setEngine(getCurrentSearchEngine());
	}
}

QString SearchWidget::getCurrentSearchEngine() const
{
	return itemData(currentIndex(), (Qt::UserRole + 1)).toString();
}

}
