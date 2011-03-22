// -*- coding: iso-8859-1 -*-
/*
 *   Author: Farhad Hedayati Fard <hf.farhad@gmail.com>
 *   Date: Thu Mar 17 2011, 19:54:29
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

Item {
  id: mainwindow
//   property string provider
//   property string from
//   property string to
  
  Component.onCompleted: {
    plasmoid.addEventListener('ConfigChanged', configChanged);
  }
  
  function configChanged()
  {
//     provider = plasmoid.readConfig("provider");
//     from = plasmoid.readConfig("fromLanguage");
//     to = plasmoid.readConfig("toLanguage");
  }

  PlasmaCore.DataSource {
    id: source
    engine: "org.kde.translator"
    onDataChanged: {
      plasmoid.busy = false
      console.log("\ndataChanged\n")
    }
    
  }
  
  PlasmaCore.DataModel {
    id: dataModel
    dataSource: source
  }
  
  PlasmaCore.Theme {
    id: theme
  }
  
  Column {
    width: mainwindow.width
    height: mainwindow.height
    Row {
      id: searchWidget
      width: parent.width
      PlasmaWidgets.LineEdit {
        id: searchBox
        clearButtonShown: true
        width: parent.width
        onTextEdited: {
            timer.running = true
            console.log("\ntext edited\n\n")
        }
      }
    }
  
    Flickable {
      clip: true
      Text {
        id: translation
        wrapMode: Text.Wrap
        width: parent.parent.width
        text: {
            var t = source.data["googletranslate" + ":" + "en" + ":" + "fa" + ":" + searchBox.text]["text"]
            console.log("text: ", source.data["googletranslate" + ":" + "en" + ":" + "fa" + ":" + searchBox.text]["text"])
            t
        }
      }
    }
  }
  
  Timer {
    id: timer
    running: false
    repeat: false
    interval: 1000
    onTriggered: {
      plasmoid.busy = true
      source.connectedSources = ["googletranslate" + ":" + "en" + ":" + "fa" + ":" + searchBox.text]
    }
  }
}