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

#include <cassert>
#include <dbus-types.h>
#include <menumodel-cpp/action.h>
#include <menumodel-cpp/action-group-exporter.h>
#include <menumodel-cpp/action-group.h>
#include <menumodel-cpp/menu-exporter.h>
#include <menumodel-cpp/menu.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <QSignalSpy>
#include <qmenumodel/unitymenumodel.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;

namespace
{

class TestMenuExporter : public Test
{
protected:
    void
    SetUp ()
    {
        Variant::registerMetaTypes();
        sessionBus = make_shared<SessionBus>();
        g_dbus_connection_set_exit_on_close(sessionBus->bus().get(), FALSE);
        actionGroup = make_shared<ActionGroup>();
        menu = make_shared<Menu>();
    }

    static QStringList
    toStringList (const QAbstractListModel &model)
    {
        QStringList result;
        int max(model.rowCount());
        for (int i(0); i < max; ++i)
        {
            result
                    << model.data(model.index(i), Qt::DisplayRole + 1).toString();
        }
        return result;
    }

    DBusTestRunner dbus;

    std::shared_ptr<SessionBus> sessionBus;

    ActionGroup::Ptr actionGroup;

    Menu::Ptr menu;

    std::unique_ptr<ActionGroupExporter> actionGroupExporter;

    std::unique_ptr<MenuExporter> menuExporter;
}
;

TEST_F(TestMenuExporter, ExportBasicActionsAndMenu)
{
    actionGroup->add(make_shared< ::Action>("apple"));
    actionGroup->add(make_shared< ::Action>("banana"));
    actionGroup->add(make_shared< ::Action>("coconut"));
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Apple", "prefix.apple"));
    menu->append(make_shared<MenuItem>("Banana", "prefix.banana"));
    menu->append(make_shared<MenuItem>("Coconut", "prefix.coconut"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    UnityMenuModel menuModel;
    QSignalSpy menuSpy(&menuModel,
                       SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    menuModel.setBusName(sessionBus->address().c_str());
    menuModel.setMenuObjectPath("/menus/path");
    QVariantMap actions;
    actions["prefix"] = "/actions/path";
    menuModel.setActions(actions);

    menuSpy.wait();
    ASSERT_FALSE(menuSpy.isEmpty());
    EXPECT_EQ(QStringList() << "Apple" << "Banana" << "Coconut",
              toStringList(menuModel));
}

TEST_F(TestMenuExporter, ActionActivation)
{
    std::shared_ptr< ::Action> apple = make_shared< ::Action>("apple");
    actionGroup->add(apple);
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Apple", "app.apple"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    UnityMenuModel menuModel;
    QSignalSpy menuSpy(&menuModel,
                       SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    menuModel.setBusName(sessionBus->address().c_str());
    menuModel.setMenuObjectPath("/menus/path");
    QVariantMap actions;
    actions["app"] = "/actions/path";
    menuModel.setActions(actions);
    menuSpy.wait();
    ASSERT_FALSE(menuSpy.isEmpty());
    ASSERT_EQ(1, menuModel.rowCount());

    QSignalSpy signalSpy(apple.get(),
                         SIGNAL(activated(const Variant&)));

    menuModel.activate(0);

    ASSERT_TRUE(signalSpy.wait());
    auto v = qvariant_cast<Variant>(signalSpy.first().first());
    EXPECT_EQ("null", v.to_string());
}

TEST_F(TestMenuExporter, ParameterizedActionActivation)
{
    std::shared_ptr< ::Action> parameterized = make_shared< ::Action>("param", G_VARIANT_TYPE_STRING);
    actionGroup->add(parameterized);
    actionGroupExporter.reset(
            new ActionGroupExporter(sessionBus, actionGroup, "/actions/path"));

    menu->append(make_shared<MenuItem>("Param", "app.param"));
    menuExporter.reset(new MenuExporter(sessionBus, "/menus/path", menu));

    UnityMenuModel menuModel;
    QSignalSpy menuSpy(&menuModel,
                       SIGNAL(rowsInserted(const QModelIndex&, int, int)));

    menuModel.setBusName(sessionBus->address().c_str());
    menuModel.setMenuObjectPath("/menus/path");
    QVariantMap actions;
    actions["app"] = "/actions/path";
    menuModel.setActions(actions);
    menuSpy.wait();
    ASSERT_FALSE(menuSpy.isEmpty());
    ASSERT_EQ(1, menuModel.rowCount());

    QSignalSpy signalSpy(parameterized.get(),
                             SIGNAL(activated(const Variant&)));

    menuModel.activate(0, "hello");

    ASSERT_TRUE(signalSpy.wait());

    auto v = qvariant_cast<Variant>(signalSpy.first().first());
    EXPECT_EQ("hello", v.as<string>());
}

} // namespace
