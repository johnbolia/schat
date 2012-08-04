/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright © 2008-2012 IMPOMEZIA <schat@impomezia.com>
 * Copyright © 2012 Alexey Ivanov <alexey.ivanes@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTimer>
#include <QMenu>

#include "ChatCore.h"
#include "ChatSettings.h"
#include "Path.h"
#include "sglobal.h"
#include "SpellChecker.h"
#include "SpellCheckerPage.h"
#include "SpellHighlighter.h"
#include "Translation.h"
#include "ui/InputWidget.h"
#include "ui/SendWidget.h"

SpellChecker* SpellChecker::m_self = 0;

SpellChecker::SpellChecker(QObject *parent)
  : ChatPlugin(parent)
  , m_textEdit(0)
  , m_position(0)
  , m_menuAction(0)
  , m_highlighter(0)
{
  m_self = this;
  m_menu = new QMenu();

  ChatCore::settings()->setLocalDefault(LS("SpellChecker/Dictionaries"), QStringList());
  ChatCore::settings()->setLocalDefault(LS("SpellChecker/Active"), true);
  ChatCore::settings()->setLocalDefault(LS("SpellChecker/Advanced"), false);
//  SettingsTabHook::add(new SpellCheckerPageCreator());

  ChatCore::translation()->addOther(LS("spellchecker"));

  QTimer::singleShot(0, this, SLOT(start()));
}


SpellChecker::~SpellChecker()
{
  delete m_menu;
}


QString SpellChecker::path()
{
# if defined(Q_WS_X11)
  return LS("/usr/share/hunspell/");
# else
  return Path::data(Path::SystemScope) + LS("/spelling");
# endif
}


/*!
 * Определение списка необходимых словарей в зависимости от языка интерфейса чата.
 */
QStringList SpellChecker::detect()
{
  QStringList dictionaries;
  QString lang = ChatCore::translation()->name();
  if (!lang.startsWith(LS("en")))
    dictionaries.append(LS("en_US"));

  QLocale locale(ChatCore::translation()->name());
  dictionaries.append(locale.name());
  return dictionaries;
}


/*!
 * Загрузка словарей в зависимости от настроек.
 *
 * - "SpellChecker/Active"   Значение \b false отключает проверку орфографии.
 * - "SpellChecker/Advanced" Значение \b true включает использование опции "SpellChecker/Dictionaries", если \b false производится попытка автоматического определения языка.
 */
void SpellChecker::reload()
{
  if (!ChatCore::settings()->value(LS("SpellChecker/Active")).toBool()) {
    SpellBackend::instance()->setLangs(QStringList());
    return;
  }

  if (ChatCore::settings()->value(LS("SpellChecker/Advanced")).toBool()) {
    QStringList dictionaries = ChatCore::settings()->value(LS("SpellChecker/Dictionaries")).toStringList();
    SpellBackend::instance()->setLangs(dictionaries);
  }
  else
    SpellBackend::instance()->setLangs(detect());
}


void SpellChecker::contextMenu(QMenu *menu, const QPoint &pos)
{
  menu->addSeparator();

  QTextCursor cursor = m_textEdit->cursorForPosition(pos);
  m_position = cursor.position();
  cursor.select(QTextCursor::WordUnderCursor);
  QString word = cursor.selectedText();

  if (word.isEmpty() || SpellBackend::instance()->isCorrect(word))
    return;

  suggestionsMenu(word, menu);
//  menu->addAction(tr("Add to dictionary"), this, SLOT(addWordToDict()));
}


/*!
 * Замена слова.
 */
void SpellChecker::repairWord()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  QTextCursor cursor = m_textEdit->textCursor();

  cursor.beginEditBlock();
  cursor.setPosition(m_position, QTextCursor::MoveAnchor);
  cursor.select(QTextCursor::WordUnderCursor);
  cursor.removeSelectedText();
  cursor.insertText(action->text());
  cursor.endEditBlock();

  m_highlighter->rehighlightBlock(cursor.block());
}


void SpellChecker::setEnabledDicts(QList<QString> &dicts)
{
  if (m_highlighter)
    delete m_highlighter;

  SpellBackend::instance()->setLangs(dicts);
  m_highlighter = new SpellHighlighter(m_textEdit->document());
  m_highlighter->rehighlight();
}


void SpellChecker::addWordToDict()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  QTextCursor cursor = m_textEdit->textCursor();
  cursor.setPosition(m_position, QTextCursor::MoveAnchor);
  cursor.select(QTextCursor::WordUnderCursor);
  const QString word = cursor.selectedText();

  SpellBackend::instance()->add(word);

  m_highlighter->rehighlightBlock(cursor.block());
}


void SpellChecker::resetMenu()
{
  m_menuAction = 0;
}


/*!
 * Инициализация.
 */
void SpellChecker::start()
{
  m_textEdit = SendWidget::i()->input();
  connect(m_textEdit, SIGNAL(contextMenu(QMenu*,QPoint)), this, SLOT(contextMenu(QMenu*,QPoint)));

  reload();
  connect(SpellBackend::instance(), SIGNAL(suggestionsReady(QString,QStringList)), SLOT(suggestions(QString,QStringList)));

  m_highlighter = new SpellHighlighter(m_textEdit->document());
}


void SpellChecker::suggestions(const QString &word, const QStringList &words)
{
  if (!m_menuAction || m_menuAction->data() != word)
    return;

  foreach(const QString &word, words) {
    m_menu->addAction(word, this, SLOT(repairWord()));
  }

  m_menuAction->setEnabled(true);
}


void SpellChecker::suggestionsMenu(const QString &word, QMenu *parent)
{
  SpellBackend::instance()->queuedSuggestions(word);
  m_menu->clear();
  m_menu->setTitle(tr("Suggestions"));
  m_menuAction = parent->addMenu(m_menu);
  m_menuAction->setEnabled(false);
  m_menuAction->setData(word);

  connect(parent, SIGNAL(aboutToHide()), SLOT(resetMenu()));
}

