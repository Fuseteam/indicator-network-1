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

#include "notification.h"

#include <libnotify/notify.h>
#include <QDebug>

using namespace notify;

class notify::Notification::Private
{
public:
    Private(Notification& parent) :
        p(parent)
    {
    }

    Notification& p;

    QString m_summary;
    QString m_body;
    QString m_icon;

    std::unique_ptr<NotifyNotification, GObjectDeleter> m_notification;

    static void closed_cb(NotifyNotification *,
                          gpointer user_data)
    {
        Private *that = static_cast<Private *>(user_data);
        Q_EMIT that->p.closed();
    }

    gulong disconnectId = 0;
};

Notification::Notification(const QString &summary,
                           const QString &body,
                           const QString &icon)
{
    d.reset(new Private(*this));
    d->m_summary = summary;
    d->m_body = body;
    d->m_icon = icon;

    d->m_notification.reset(notify_notification_new(summary.toUtf8().constData(), body.toUtf8().constData(), icon.toUtf8().constData()));
    d->disconnectId = g_signal_connect(d->m_notification.get(), "closed", G_CALLBACK(Private::closed_cb), d.get());
}

Notification::~Notification()
{
    g_signal_handler_disconnect(d->m_notification.get(), d->disconnectId);

// TODO Uncomment this when the notification service is more robust
//    GError* error = nullptr;
//    if (notify_notification_close(d->m_notification.get(), &error) == FALSE)
//    {
//        qWarning() << __PRETTY_FUNCTION__ << error->message;
//        g_error_free(error);
//    }
}

QString
Notification::summary()
{
    return d->m_summary;
}

QString
Notification::body()
{
    return d->m_body;
}

QString
Notification::icon()
{
    return d->m_icon;
}

void
Notification::setHint(const QString &key, Variant value)
{
    notify_notification_set_hint(d->m_notification.get(), key.toUtf8().constData(), value);
}

void
Notification::setHintString(const QString &key, const QString &value)
{
    notify_notification_set_hint_string(d->m_notification.get(), key.toUtf8().constData(), value.toUtf8().constData());
}

void
Notification::setSummary(const QString& summary)
{
    if (d->m_summary == summary)
    {
        return;
    }

    d->m_summary = summary;
    g_object_set(d->m_notification.get(), "summary", d->m_summary.toUtf8().constData(), nullptr);
    Q_EMIT summaryUpdated(d->m_summary);
}

void
Notification::setBody(const QString& body)
{
    if (d->m_body == body)
    {
        return;
    }

    d->m_body = body;
    g_object_set(d->m_notification.get(), "body", d->m_body.toUtf8().constData(), nullptr);
    Q_EMIT bodyUpdated(d->m_body);
}

void
Notification::setIcon(const QString& icon)
{
    if (d->m_icon == icon)
    {
        return;
    }

    d->m_icon = icon;
    g_object_set(d->m_notification.get(), "icon", d->m_icon.toUtf8().constData(), nullptr);
    Q_EMIT iconUpdated(d->m_icon);
}

void
Notification::update()
{
    notify_notification_update(d->m_notification.get(), d->m_summary.toUtf8().constData(),
                               d->m_body.toUtf8().constData(), d->m_icon.toUtf8().constData());
}

void
Notification::show()
{
    GError *error = nullptr;
    notify_notification_show(d->m_notification.get(), &error);
    if (error) {
        qCritical() << __PRETTY_FUNCTION__ << error->message;
        g_error_free(error);
    }
}

void
Notification::close()
{
    GError *error = nullptr;
    notify_notification_close(d->m_notification.get(), &error);
    if (error) {
        qCritical() << __PRETTY_FUNCTION__ << error->message;
        g_error_free(error);
    }
}
