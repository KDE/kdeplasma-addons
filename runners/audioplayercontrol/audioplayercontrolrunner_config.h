/*
 *   Copyright (C) 2009 Jan G. Marker <jangerrit@weiler-marker.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef AUDIOPLAYERCONTROLRUNNERCONFIG_H
#define AUDIOPLAYERCONTROLRUNNERCONFIG_H

//Project-Includes
#include "ui_audioplayercontrolrunner_config.h"
//Qt-Includes
//KDE-Includes
#include <KCModule>

class AudioPlayerControlRunnerConfigForm : public QWidget, public Ui::AudioPlayerControlRunnerUI
{
    Q_OBJECT

public:
    explicit AudioPlayerControlRunnerConfigForm(QWidget* parent);
};

class AudioPlayerControlRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit AudioPlayerControlRunnerConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());

public slots:
    void save();
    void load();
    void defaults();

private:
    AudioPlayerControlRunnerConfigForm* m_ui;

private slots:
    /** Is called, if the current index of the player combobox changed */
    void onPlayerChanged(int index);
    void onPlayerChanged(QString text);
};
#endif // AUDIOPLAYERCONTROLRUNNERCONFIG_H
