/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Antti Kaijanmäki <antti.kaijanmaki@canonical.com>
 */

#ifndef NOTIFY_NOTIFICATION_H
#define NOTIFY_NOTIFICATION_H

#include <memory>

#include "menumodel-cpp/gio-helpers/variant.h"

#include <QObject>

namespace notify {

class Notification: public QObject
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:

    typedef std::shared_ptr<Notification> Ptr;

    Notification() = delete;
    Notification(const QString &summary,
                 const QString &body,
                 const QString &icon);
    virtual ~Notification();

    /// @todo remember show() after set().
    Q_PROPERTY(QString summary READ summary NOTIFY summaryUpdated)
    QString summary();

    /// @todo remember show() after set().
    Q_PROPERTY(QString body READ body NOTIFY bodyUpdated)
    QString body();

    /// @todo remember show() after set().
    Q_PROPERTY(QString icon READ icon NOTIFY iconUpdated)
    QString icon();

    void update();

    void show();
    void close();

public Q_SLOTS:
    void setHint(const QString &key, Variant value);

    void setHintString(const QString &key, const QString &value);

    void setSummary(const QString& summary);

    void setBody(const QString& body);

    void setIcon(const QString& icon);

Q_SIGNALS:
    void closed();

    void summaryUpdated(const QString&);

    void bodyUpdated(const QString&);

    void iconUpdated(const QString&);
};
}

#endif // NOTIFY_NOTIFICATION_H
