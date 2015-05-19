/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "sim-unlock-dialog.h"

#include <notify-cpp/notification.h>
#include <notify-cpp/snapdecision/sim-unlock.h>
#include <util/localisation.h>

#include <functional>
#include <sstream>
#include <QDebug>

using namespace std;
using namespace nmofono;

namespace ubuntu {
namespace i18n{

std::string __argumentSubstitute(int /*depth*/, const std::string &format) // base function
{
    return format;
}

template<typename T, typename... Targs>
std::string __argumentSubstitute(int depth, const std::string &format, T value, Targs... Fargs)
{
    std::string tmp = __argumentSubstitute(depth+1, format, Fargs...);
    std::string subs = "%{" + std::to_string(depth) + "}";

    const std::string val {value};
    for (auto it = std::search(tmp.begin(), tmp.end(), subs.begin(), subs.end());
         it != tmp.end();
         it = std::search(tmp.begin(), tmp.end(), subs.begin(), subs.end()))
    {
        auto pos = it;
        pos = tmp.erase(pos, pos+subs.length());
        tmp.insert(pos, val.begin(), val.end());
    }
    return tmp;
}

/// substitute variables in strings by index
/// only accepts std::string as input for now
/// argumentSubstitute("First: %{1}, Second %{2}", std::string{"foo"}, std::string{"bar"});
///     --> "First: foo, Second bar"
/// argumentSubstitute("First: %{2}, Second %{1}", std::string{"foo"}, std::string{"bar"});
///     --> "First: bar, Second: foor"
template<typename... Targs>
std::string argumentSubstitute(const std::string &format, Targs... Fargs)
{
    return __argumentSubstitute(1, format, Fargs...);
}
}
}

class SimUnlockDialog::Private: public QObject
{
    Q_OBJECT

public:
    enum class Mode
    {
        enterPin,
        resetPin,
        lockPin,
        unlockPin
    };

    enum class EnterPinStates
    {
        initial,
        enterPin,
        enterPuk,
        enterNewPin,
        confirmNewPin
    };

    SimUnlockDialog& p;

    EnterPinStates m_enterPinState;

    State m_state = State::ready;
    wwan::Modem::Ptr m_modem;

    notify::snapdecision::SimUnlock::Ptr m_sd;

    QString m_newPin;
    QString m_oldPin;
    QString m_pukCode;

    std::vector<QMetaObject::Connection> m_connections;

    bool m_showSimIdentifiers = false;

    /// @todo see comment in reset()
    bool m_doCleanUp;

    int m_pinRetries = -1;

    int m_pukRetries = -1;

    void sendEnterPin(const QString& pin)
    {
        m_pinRetries = -1;
        auto retriesMap = m_modem->retries();
        if (retriesMap.find(wwan::Modem::PinType::pin) != retriesMap.end())
        {
            m_pinRetries = retriesMap[wwan::Modem::PinType::pin];
        }
        m_modem->enterPin(wwan::Modem::PinType::pin, pin);
    }

    void sendResetPin(const QString& puk, const QString& newPin)
    {
        m_pukRetries = -1;
        auto retriesMap = m_modem->retries();
        if (retriesMap.find(wwan::Modem::PinType::puk) != retriesMap.end())
        {
            m_pukRetries = retriesMap[wwan::Modem::PinType::puk];
        }
        m_modem->resetPin(wwan::Modem::PinType::puk, puk, newPin);
    }

    void showLastPinAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(_("Sorry, incorrect %{1} PIN."),
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        output << " ";
        output << _("This will be your last attempt.");
        output << " ";
        output << ubuntu::i18n::argumentSubstitute(_("If %{1} PIN is entered incorrectly you will require your PUK code to unlock."),
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        m_sd->showPopup(output.str(), closed);
    }

    void showPinBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << ubuntu::i18n::argumentSubstitute(std::string{_("Sorry, your %{1} is now blocked.")},
                                                   m_showSimIdentifiers ?
                                                       m_modem->simIdentifier().toStdString()
                                                     : "SIM");
        output << " ";
        output << _("Please enter your PUK code to unblock SIM card.");
        output << " ";
        output << _("You may need to contact your network provider for PUK code.");

        m_sd->showPopup(output.str(), closed);
    }

    void showLastPukAttemptPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << " ";
        output << _("This will be your last attempt.");
        output << " ";
        output << _("If PUK code is entered incorrectly, your SIM card will be blocked and needs replacement.");
        output << " ";
        output << _("Please contact your network provider.");

        m_sd->showPopup(output.str(), closed);
    }

    void showSimPermanentlyBlockedPopup(std::function<void()> closed = std::function<void()>())
    {
        std::stringstream output;
        output << _("Sorry, incorrect PUK.");
        output << " ";
        output << _("Your SIM card is now permanently blocked and needs replacement.");
        output << " ";
        output << _("Please contact your service provider.");

        m_sd->showPopup(output.str(), closed);
    }

    Private(SimUnlockDialog& parent);

public Q_SLOTS:
    void update();

    void cancelled();

    void pinEntered(const QString& pin);

    void closed();

    void reset();

    void pinSuccess()
    {
        m_sd->close();
        reset();
    }

    void enterPinFailed(const QString&)
    {
        m_sd->showError(_("Sorry, incorrect PIN"));
        --m_pinRetries;
        if (m_pinRetries == 1) {
            showLastPinAttemptPopup();
        } else if (m_pinRetries == 0) {
            showPinBlockedPopup();
        }
        update();
    }

    void resetPinFailed(const QString&)
    {
        m_sd->showError(_("Sorry, incorrect PUK"));
        --m_pukRetries;
        if (m_pukRetries == 1) {
            showLastPukAttemptPopup();
        } else if (m_pukRetries == 0) {
            showSimPermanentlyBlockedPopup([this](){ m_sd->close(); });
        }

        m_enterPinState = EnterPinStates::enterPuk;
        m_pukCode.clear();
        m_newPin.clear();
        update();
    }
};

SimUnlockDialog::Private::Private(SimUnlockDialog& parent)
    : p(parent), m_doCleanUp{true}
{
    m_sd = std::make_shared<notify::snapdecision::SimUnlock>();
    connect(m_sd.get(), &notify::snapdecision::SimUnlock::cancelled, this, &Private::cancelled);
    connect(m_sd.get(), &notify::snapdecision::SimUnlock::pinEntered, this, &Private::pinEntered);
    connect(m_sd.get(), &notify::snapdecision::SimUnlock::closed, this, &Private::closed);

    reset();
}

void
SimUnlockDialog::Private::update()
{
    if (!m_modem || !m_sd)
    {
        return;
    }

    m_sd->setTitle("");
    m_sd->setBody("");
    m_sd->setPinMinMax({0, 0});

    std::map<wwan::Modem::PinType, std::pair<std::uint8_t, std::uint8_t>> lengths;
    lengths = {{wwan::Modem::PinType::pin, {4, 8}},
               {wwan::Modem::PinType::puk, {8, 8}}};

    auto type = m_modem->requiredPin();
    auto retries = m_modem->retries();

    if (m_enterPinState == EnterPinStates::enterPin &&
        type == wwan::Modem::PinType::puk) {
        // we transitioned from pin query to PUK query
        m_enterPinState = EnterPinStates::enterPuk;
    }

    switch (m_enterPinState) {
    case EnterPinStates::initial:
        return;
    case EnterPinStates::enterPin:
    case EnterPinStates::enterPuk:
    {
        std::string title;
        switch(type){
        case wwan::Modem::PinType::none:
            // we are done.
            return;
        case wwan::Modem::PinType::pin:
            title = ubuntu::i18n::argumentSubstitute(_("Enter %{1} PIN"),
                                                     m_showSimIdentifiers ?
                                                         m_modem->simIdentifier().toStdString()
                                                       : "SIM");
            break;
        case wwan::Modem::PinType::puk:
            if (!m_showSimIdentifiers)
            {
                title = _("Enter PUK code");
            }
            else
            {
                title = ubuntu::i18n::argumentSubstitute(_("Enter PUK code for %{1}"),
                                                           m_modem->simIdentifier().toStdString());
            }
            break;
        }

        m_sd->setPinMinMax(lengths[type]);

        std::string attempts;
        if (retries.find(type) != retries.end()) {
            gchar *tmp = g_strdup_printf(ngettext("1 attempt remaining", "%d attempts remaining", retries[type]), retries[type]);
            attempts = {tmp};
            g_free(tmp);
        }

        m_sd->setTitle(QString::fromStdString(title));
        m_sd->setBody(QString::fromStdString(attempts));
        break;
    }
    case EnterPinStates::enterNewPin:
        m_sd->setBody("Create new PIN");
        m_sd->setTitle(QString::fromStdString(ubuntu::i18n::argumentSubstitute(_("Enter new %{1} PIN"),
                                                           m_showSimIdentifiers ?
                                                               m_modem->simIdentifier().toStdString()
                                                             : "SIM")));
        m_sd->setPinMinMax(lengths[wwan::Modem::PinType::pin]);
        break;
    case EnterPinStates::confirmNewPin:
        m_sd->setBody("Create new PIN");
        m_sd->setTitle(QString::fromStdString(ubuntu::i18n::argumentSubstitute(_("Confirm new %{1} PIN"),
                                                           m_showSimIdentifiers ?
                                                               m_modem->simIdentifier().toStdString()
                                                             : "SIM")));
        m_sd->setPinMinMax(lengths[wwan::Modem::PinType::pin]);
        break;
    }


    /// @todo should be able to see cleartext puk and pin when entering puk or changing pin.
    m_sd->update();
    m_sd->show();
}



void
SimUnlockDialog::Private::pinEntered(const QString& pin)
{
    switch (m_enterPinState) {
    case EnterPinStates::initial:
        // should never happen
        assert(0);
        return;
    case EnterPinStates::enterPin:
        sendEnterPin(pin);
        break;
    case EnterPinStates::enterPuk:
        m_pukCode = pin;
        m_enterPinState = EnterPinStates::enterNewPin;
        update();
        break;
    case EnterPinStates::enterNewPin:
        m_newPin = pin;
        m_enterPinState = EnterPinStates::confirmNewPin;
        update();
        break;
    case EnterPinStates::confirmNewPin:
        if (m_newPin != pin) {
            m_sd->showError(_("PIN codes did not match."));
            m_enterPinState = EnterPinStates::enterNewPin;
            m_newPin.clear();
            update();
        } else {
            sendResetPin(m_pukCode, pin);
        }
        break;
    }
}

void
SimUnlockDialog::Private::cancelled()
{
    m_sd->close();
}

void
SimUnlockDialog::Private::closed()
{
    reset();
}

void
SimUnlockDialog::Private::reset()
{
    if (m_doCleanUp)
    {
        for (auto &c : m_connections)
        {
            disconnect(c);
        }
        m_connections.clear();
    }

    m_modem.reset();

    m_newPin.clear();
    m_oldPin.clear();
    m_pukCode.clear();

    m_enterPinState = EnterPinStates::initial;

    m_state = State::ready;
    m_doCleanUp = false;
    Q_EMIT p.ready();
    m_doCleanUp = true;
}

SimUnlockDialog::SimUnlockDialog() :
        d(new Private(*this))
{
}

SimUnlockDialog::~SimUnlockDialog()
{}

/// must not be called is unlocking already in progress
void
SimUnlockDialog::unlock(wwan::Modem::Ptr modem)
{
    if (d->m_modem)
    {
        throw std::logic_error("Unlocking already in progress.");
    }

    d->m_modem = modem;
    d->m_state = State::unlocking;

    auto retries = modem->retries();
    auto type = modem->requiredPin();
    switch(type){
    case wwan::Modem::PinType::none:
        d->reset();
        return;
    case wwan::Modem::PinType::pin:
        d->m_enterPinState = Private::EnterPinStates::enterPin;
        break;
    case wwan::Modem::PinType::puk:
        d->m_enterPinState = Private::EnterPinStates::enterPuk;
        break;
    }

    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::requiredPinUpdated, d.get(), &Private::update));
    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::retriesUpdated, d.get(), &Private::update));
    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::enterPinSuceeded, d.get(), &Private::pinSuccess));
    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::resetPinSuceeded, d.get(), &Private::pinSuccess));
    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::enterPinFailed, d.get(), &Private::enterPinFailed));
    d->m_connections.emplace_back(connect(modem.get(), &wwan::Modem::resetPinFailed, d.get(), &Private::resetPinFailed));

    int pinRetries = -1;
    int pukRetries = -1;

    if (retries.find(wwan::Modem::PinType::pin) != retries.end())
    {
        pinRetries = retries[wwan::Modem::PinType::pin];
    }
    if (retries.find(wwan::Modem::PinType::puk) != retries.end())
    {
        pukRetries = retries[wwan::Modem::PinType::puk];
    }

    // remind the user
    if (type == wwan::Modem::PinType::pin && pinRetries == 1)
    {
        d->showLastPinAttemptPopup();
    }
    else if (type == wwan::Modem::PinType::puk) {
        // we we know the sim is permanently blocked, just show the notification straight away
        if (pukRetries == 0)
        {
            d->showSimPermanentlyBlockedPopup([this](){ cancel(); });
        }
        else
        {
            d->showPinBlockedPopup([this, pukRetries](){
                if (pukRetries == 1) {
                    d->showLastPukAttemptPopup();
                }
            });
        }
    }

    d->update();
}

void
SimUnlockDialog::cancel()
{
    d->m_sd->close();
}

wwan::Modem::Ptr
SimUnlockDialog::modem()
{
    return d->m_modem;
}

SimUnlockDialog::State
SimUnlockDialog::state() const
{
    return d->m_state;
}

bool
SimUnlockDialog::showSimIdentifiers() const
{
    return d->m_showSimIdentifiers;
}

void SimUnlockDialog::setShowSimIdentifiers(bool showSimIdentifiers)
{
    if (d->m_showSimIdentifiers == showSimIdentifiers)
    {
        return;
    }

    d->m_showSimIdentifiers = showSimIdentifiers;
    Q_EMIT showSimIdentifiersUpdated(d->m_showSimIdentifiers);
}

#include "sim-unlock-dialog.moc"
