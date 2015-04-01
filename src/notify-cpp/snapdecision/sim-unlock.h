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

#ifndef SIM_UNLOCK_DIALOG_H
#define SIM_UNLOCK_DIALOG_H

#include <memory>

#include <QObject>
#include <QString>

namespace notify {
namespace snapdecision {

class SimUnlock: public QObject
{
    Q_OBJECT

    class Private;
    std::unique_ptr<Private> d;

public:

    typedef std::shared_ptr<SimUnlock> Ptr;

    explicit SimUnlock(const QString &title = "",
              const QString &body = "",
              std::pair<std::uint8_t, std::uint8_t> pinMinMax = {0, 0});
    ~SimUnlock();

    /**
     * To update the value in the dialog, call update().
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleUpdated)
    QString title();

    /**
     * To update the value in the dialog, call update().
     */
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyUpdated)
    QString body();

    /**
     * To update the value in the dialog, call update().
     */
//    Q_PROPERTY(std::pair<std::uint8_t, std::uint8_t> pinMinMax READ pinMinMax WRITE setPinMinMax NOTIFY pinMinMaxUpdated)
    std::pair<std::uint8_t, std::uint8_t> pinMinMax();

    /**
     * Update the dialog.
     * if the dialog has not been shown, does nothing.
     */
    void update();

    void show();
    void close();

    void showError(std::string message, std::function<void()> closed = std::function<void()>());
    void showPopup(std::string message, std::function<void()> closed = std::function<void()>());

public Q_SLOTS:
    void setTitle(const QString& title);

    void setBody(const QString& body);

    void setPinMinMax(const std::pair<std::uint8_t, std::uint8_t>& pinMinMax);

Q_SIGNALS:
    void pinEntered(const QString&);

    void cancelled();

    void closed();

    void titleUpdated(const QString&);

    void bodyUpdated(const QString&);

    void pinMinMaxUpdated(const std::pair<std::uint8_t, std::uint8_t>&);
};

}
}
#endif
