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

#include <nmofono/wwan/qofono-sim-wrapper.h>

class QOfonoModem;

namespace nmofono
{
class ManagerImpl;

namespace wwan
{

class Sim : public QObject
{
    Q_OBJECT

    class Private;
    std::shared_ptr<Private> d;

public:

    typedef std::shared_ptr<Sim> Ptr;
    typedef std::weak_ptr<Sim> WeakPtr;

    enum class PinType
    {
        none,
        pin,
        puk
    };

    enum class Status
    {
        missing,
        error,
        locked,
        permanentlyLocked,
        ready,
        not_available
    };

    Sim() = delete;

    static Sim::Ptr createFromSettings(QSettings*, const QString &imsi);
    static void saveToSettings(QSettings *, Sim::Ptr);

    static Sim::Ptr fromQOfonoSimWrapper(const QOfonoSimWrapper *wrapper);

private:
    Sim(const QString &imsi,
        const QString &primaryPhoneNumber,
        const QString &mcc,
        const QString &mnc,
        const QStringList &preferredLanguages,
        bool dataRoamingEnabled);

public:
    ~Sim();
    void setOfonoSimManager(std::shared_ptr<QOfonoSimManager> simmgr);


    Q_PROPERTY(Sim::PinType requiredPin READ requiredPin NOTIFY requiredPinUpdated)
    PinType requiredPin() const;

    typedef std::map<Sim::PinType, int> RetriesType;
    Q_PROPERTY(RetriesType retries READ retries NOTIFY retriesUpdated)
    const RetriesType &retries() const;

    Q_PROPERTY(QString simIdentifier READ simIdentifier NOTIFY simIdentifierUpdated)
    const QString &simIdentifier() const;

    Q_PROPERTY(Sim::Status status READ status NOTIFY statusUpdated)
    Status status() const;

    Q_PROPERTY(QString imsi READ imsi CONSTANT)
    QString imsi() const;

    Q_PROPERTY(QString primaryPhoneNumber READ primaryPhoneNumber CONSTANT)
    QString primaryPhoneNumber() const;

    Q_PROPERTY(bool locked READ locked NOTIFY lockedChanged)
    bool locked() const;

    Q_PROPERTY(bool present READ present NOTIFY presentChanged)
    bool present() const;

    Q_PROPERTY(QString mcc READ mcc CONSTANT)
    QString mcc() const;

    Q_PROPERTY(QString mnc READ mnc CONSTANT)
    QString mnc() const;

    Q_PROPERTY(QList<QString> preferredLanguages READ preferredLanguages CONSTANT)
    QList<QString> preferredLanguages() const;

    Q_PROPERTY(bool dataRoamingEnabled READ dataRoamingEnabled WRITE setDataRoamingEnabled NOTIFY dataRoamingEnabledChanged)
    bool dataRoamingEnabled() const;
    void setDataRoamingEnabled(bool value);

    Q_PROPERTY(bool mobileDataEnabled READ mobileDataEnabled WRITE setMobileDataEnabled NOTIFY mobileDataEnabledChanged)
    bool mobileDataEnabled() const;
    void setMobileDataEnabled(bool value);

    bool isReadyToUnlock() const;

    void notifyWhenReadyToUnlock();

    void enterPin(PinType type,
                  const QString &pin);

    void resetPin(PinType type,
                  const QString &puk,
                  const QString &pin);

    QString ofonoPath() const;

public Q_SLOTS:
    void unlock();

Q_SIGNALS:

    void requiredPinUpdated(PinType);

    void retriesUpdated();

    void simIdentifierUpdated(const QString &);

    void updated(const Sim& sim);

    void enterPinSuceeded();

    void enterPinFailed(const QString& error);

    void resetPinSuceeded();

    void resetPinFailed(const QString& error);

    bool readyToUnlock(const QString& name);

    void statusUpdated();

    void lockedChanged(bool value);

    void presentChanged(bool value);

    void dataRoamingEnabledChanged(bool value);

    void mobileDataEnabledChanged(bool value);
};

}
}
