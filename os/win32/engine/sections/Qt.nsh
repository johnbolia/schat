/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright � 2008 IMPOMEZIA <schat@impomezia.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

${Name} Qt

${Body}
${Section}
  SectionIn RO

  SetOutPath "$INSTDIR"
  File "${SCHAT_QTDIR}\bin\QtCore4.dll"
  File "${SCHAT_QTDIR}\bin\QtGui4.dll"
  File "${SCHAT_QTDIR}\bin\QtNetwork4.dll"

  !if ${SCHAT_VC90} == 1
    File "${VC90_REDIST_DIR}\msvcr90.dll"
    File "contrib\Microsoft.VC90.CRT.manifest"
  !endif

  SetOutPath "$INSTDIR\plugins\imageformats"
  File "${SCHAT_QTDIR}\plugins\imageformats\qgif4.dll"
${SectionEnd}
${BodyEnd}