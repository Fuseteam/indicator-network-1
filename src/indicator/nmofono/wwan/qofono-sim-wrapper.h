/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#pragma once

#include <QObject>
#include <QSettings>

#include <memory>

#define slots
#include <qofono-qt5/qofonomodem.h>
#include <qofono-qt5/qofonosimmanager.h>
#undef slots

class QOfonoModem;

namespace nmofono
{
class ManagerImpl;

namespace wwan
{

class QOfonoSimWrapper : public QObject
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:

    typedef std::shared_ptr<QOfonoSimWrapper> Ptr;
    typedef std::weak_ptr<QOfonoSimWrapper> WeakPtr;

    QOfonoSimWrapper() = delete;


    QOfonoSimWrapper(std::shared_ptr<QOfonoSimManager> simmgr);
    ~QOfonoSimWrapper();

    QString imsi() const;
    bool present() const;
    QString mcc() const;
    QString mnc() const;
    QStringList phoneNumbers() const;
    QStringList preferredLanguages() const;
    bool ready() const;

    std::shared_ptr<QOfonoSimManager> ofonoSimManager() const;


Q_SIGNALS:

    void presentChanged(bool value);
    void readyChanged(bool value);
};

}
}
