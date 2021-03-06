/* $Id$
 * IMPOMEZIA Simple Chat
 * Copyright � 2008-2014 IMPOMEZIA <schat@impomezia.com>
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

${Name} Share
${State} 1

${Body}
${Section}
  SetOutPath "$INSTDIR\plugins"
  File "${SCHAT_SOURCE}\plugins\Share.dll"

  WriteRegStr HKCU "Software\Microsoft\Internet Explorer\Low Rights\DragDrop\{F0BDC313-72FB-40DF-B178-F38E1D965E18}" "AppName" "schat2.exe"
  WriteRegStr HKCU "Software\Microsoft\Internet Explorer\Low Rights\DragDrop\{F0BDC313-72FB-40DF-B178-F38E1D965E18}" "AppPath" "$INSTDIR"
  WriteRegDWORD HKCU "Software\Microsoft\Internet Explorer\Low Rights\DragDrop\{F0BDC313-72FB-40DF-B178-F38E1D965E18}" "Policy" 3
${SectionEnd}
${BodyEnd}

${Uninstall}
  Delete "$INSTDIR\plugins\Share.dll"
  RMDir "$INSTDIR\plugins"

  DeleteRegKey HKCU "Software\Microsoft\Internet Explorer\Low Rights\DragDrop\{F0BDC313-72FB-40DF-B178-F38E1D965E18}"
${UninstallEnd}