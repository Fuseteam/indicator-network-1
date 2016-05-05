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

#include <nmofono/wwan/qofono-sim-wrapper.h>

#include <ofono/dbus.h>

using namespace std;

namespace nmofono
{
namespace wwan
{
namespace
{

}

class QOfonoSimWrapper::Private : public QObject, public std::enable_shared_from_this<Private>
{
    Q_OBJECT

public:

    QOfonoSimWrapper& p;
    shared_ptr<QOfonoSimManager> m_simManager;

    QString m_imsi;
    QStringList m_phoneNumbers;
    QString m_mcc;
    QString m_mnc;
    QStringList m_preferredLanguages;

    bool m_present = false;

    bool m_imsiSet = false;
    bool m_phoneNumbersSet = false;
    bool m_mccSet = false;
    bool m_mncSet = false;
    bool m_preferredLanguagesSet = false;


    Private(QOfonoSimWrapper& parent, shared_ptr<QOfonoSimManager> simmgr)
        : p(parent), m_simManager{simmgr}
    {
        connect(simmgr.get(), &QOfonoSimManager::presenceChanged, this, &Private::presentChanged);
        connect(simmgr.get(), &QOfonoSimManager::subscriberIdentityChanged, this, &Private::imsiChanged);
        connect(simmgr.get(), &QOfonoSimManager::mobileCountryCodeChanged, this, &Private::mccChanged);
        connect(simmgr.get(), &QOfonoSimManager::mobileNetworkCodeChanged, this, &Private::mncChanged);
        connect(simmgr.get(), &QOfonoSimManager::subscriberNumbersChanged, this, &Private::phoneNumbersChanged);
        connect(simmgr.get(), &QOfonoSimManager::preferredLanguagesChanged, this, &Private::preferredLanguagesChanged);
    }

public Q_SLOTS:

    void presentChanged(bool value)
    {
        if (m_present == value)
        {
            return;
        }
        m_present = value;
        if (!m_present) {
            m_imsiSet = false;
            m_phoneNumbersSet = false;
            m_mccSet = false;
            m_mncSet = false;
            m_preferredLanguagesSet = false;
            Q_EMIT p.readyChanged(false);
        }
        Q_EMIT p.presentChanged(value);
    }

    void imsiChanged(const QString &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_imsiSet)
        {
            qWarning() << "Unexpected update on IMSI: " << m_imsi << ", " << value;
        }

        m_imsi = value;
        m_imsiSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }

    void mccChanged(const QString &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_mccSet)
        {
            qWarning() << "Unexpected update on MCC: " << m_mcc << ", " << value;
        }


        m_mcc = value;
        m_mccSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }

    void mncChanged(const QString &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_mncSet)
        {
            qWarning() << "Unexpected update on MNC: " << m_mnc << ", " << value;
        }

        m_mnc = value;
        m_mncSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }

    void phoneNumbersChanged(const QStringList &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_phoneNumbersSet)
        {
            qWarning() << "Unexpected update on Phone Numbers: " << m_phoneNumbers << ", " << value;
        }


        m_phoneNumbers = value;
        m_phoneNumbersSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }

    void preferredLanguagesChanged(const QStringList &value)
    {
        if (value.isEmpty())
        {
            return;
        }

        if (m_preferredLanguagesSet)
        {
            qWarning() << "Unexpected update on Preferred Languages: " << m_preferredLanguages << ", " << value;
        }


        m_preferredLanguages = value;
        m_preferredLanguagesSet = true;
        if (p.ready())
        {
            Q_EMIT p.readyChanged(true);
        }
    }
};



QOfonoSimWrapper::QOfonoSimWrapper(std::shared_ptr<QOfonoSimManager> simmgr)
    : d{new Private(*this, simmgr)}
{
}

QOfonoSimWrapper::~QOfonoSimWrapper()
{}

QString QOfonoSimWrapper::imsi() const
{
    return d->m_imsi;
}

bool QOfonoSimWrapper::present() const
{
    return d->m_present;
}

QString QOfonoSimWrapper::mcc() const
{
    return d->m_mcc;
}

QString QOfonoSimWrapper::mnc() const
{
    return d->m_mnc;
}

QStringList QOfonoSimWrapper::phoneNumbers() const
{
    return d->m_phoneNumbers;
}

QStringList QOfonoSimWrapper::preferredLanguages() const
{
    return d->m_preferredLanguages;
}

bool QOfonoSimWrapper::ready() const {
    return d->m_imsiSet &&
            d->m_phoneNumbersSet &&
            d->m_mccSet &&
            d->m_mncSet &&
            d->m_preferredLanguagesSet;
}

std::shared_ptr<QOfonoSimManager> QOfonoSimWrapper::ofonoSimManager() const
{
    return d->m_simManager;
}


}
}

#include "qofono-sim-wrapper.moc"
