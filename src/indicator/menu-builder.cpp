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

#include <menu-builder.h>
#include <factory.h>

using namespace std;

struct MenuBuilder::Priv
{
    IndicatorMenu::Ptr m_desktopMenu;
    IndicatorMenu::Ptr m_desktopGreeterMenu;

    IndicatorMenu::Ptr m_tabletMenu;
    IndicatorMenu::Ptr m_tabletGreeterMenu;

    IndicatorMenu::Ptr m_phoneMenu;
    IndicatorMenu::Ptr m_phoneGreeterMenu;

    IndicatorMenu::Ptr m_ubiquityMenu;

    RootState::Ptr m_rootState;

    QuickAccessSection::Ptr m_quickAccessSection;
    WifiSection::Ptr m_wifiSection;
    WwanSection::Ptr m_wwanSection;

    MenuExporter::UPtr m_desktopMenuExporter;
    MenuExporter::UPtr m_desktopGreeterMenuExporter;
    MenuExporter::UPtr m_desktopWifiSettingsMenuExporter;

    MenuExporter::UPtr m_phoneMenuExporter;
    MenuExporter::UPtr m_phoneGreeterMenuExporter;
    MenuExporter::UPtr m_phoneWifiSettingsMenuExporter;

    MenuExporter::UPtr m_tabletpMenuExporter;
    MenuExporter::UPtr m_tabletGreeterMenuExporter;
    MenuExporter::UPtr m_tabletWifiSettingsMenuExporter;

    MenuExporter::UPtr m_ubiquityMenuExporter;

    ActionGroupExporter::UPtr m_actionGroupExporter;
    ActionGroupMerger::UPtr m_actionGroupMerger;

    BusName::UPtr m_busName;
};

MenuBuilder::MenuBuilder(Factory& factory) :
        d(new Priv)
{
    d->m_rootState = factory.newRootState();

    d->m_desktopMenu = factory.newIndicatorMenu(d->m_rootState, "desktop");
    d->m_desktopGreeterMenu = factory.newIndicatorMenu(d->m_rootState, "desktop.greeter");

    d->m_tabletMenu = factory.newIndicatorMenu(d->m_rootState, "tablet");
    d->m_tabletGreeterMenu = factory.newIndicatorMenu(d->m_rootState, "tablet.greeter");

    d->m_phoneMenu = factory.newIndicatorMenu(d->m_rootState, "phone");
    d->m_phoneGreeterMenu = factory.newIndicatorMenu(d->m_rootState, "phone.greeter");

    d->m_ubiquityMenu = factory.newIndicatorMenu(d->m_rootState, "ubiquity");

    d->m_wifiSection = factory.newWiFiSection();
    d->m_quickAccessSection = factory.newQuickAccessSection(d->m_wifiSection->wifiSwitch());
    d->m_wwanSection = factory.newWwanSection();

    d->m_desktopMenu->addSection(d->m_quickAccessSection);
    d->m_desktopGreeterMenu->addSection(d->m_quickAccessSection);
    d->m_phoneMenu->addSection(d->m_quickAccessSection);
    d->m_phoneGreeterMenu->addSection(d->m_quickAccessSection);

    d->m_desktopMenu->addSection(d->m_wwanSection);
    d->m_desktopGreeterMenu->addSection(d->m_wwanSection);
    d->m_phoneMenu->addSection(d->m_wwanSection);
    d->m_phoneGreeterMenu->addSection(d->m_wwanSection);

    d->m_desktopMenu->addSection(d->m_wifiSection);
    d->m_desktopGreeterMenu->addSection(d->m_wifiSection);
    d->m_phoneMenu->addSection(d->m_wifiSection);
    d->m_phoneGreeterMenu->addSection(d->m_wifiSection);

    d->m_desktopMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/desktop", d->m_desktopMenu->menu());
    d->m_desktopGreeterMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/desktop_greeter", d->m_desktopGreeterMenu->menu());
    d->m_desktopWifiSettingsMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/desktop_wifi_settings", d->m_wifiSection->settingsModel());

    d->m_tabletpMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/tablet", d->m_tabletMenu->menu());
    d->m_tabletGreeterMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/tablet_greeter", d->m_tabletGreeterMenu->menu());
    d->m_tabletWifiSettingsMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/tablet_wifi_settings", d->m_wifiSection->settingsModel());

    d->m_phoneMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone", d->m_phoneMenu->menu());
    d->m_phoneGreeterMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone_greeter", d->m_phoneGreeterMenu->menu());
    d->m_phoneWifiSettingsMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/phone_wifi_settings", d->m_wifiSection->settingsModel());

    d->m_ubiquityMenuExporter = factory.newMenuExporter("/com/canonical/indicator/network/ubiquity", d->m_ubiquityMenu->menu());

    // we have a single actiongroup for all the menus.
    d->m_actionGroupMerger = factory.newActionGroupMerger();
    d->m_actionGroupMerger->add(d->m_desktopMenu->actionGroup());
    d->m_actionGroupMerger->add(d->m_desktopGreeterMenu->actionGroup());
    d->m_actionGroupMerger->add(d->m_phoneMenu->actionGroup());
    d->m_actionGroupMerger->add(d->m_phoneGreeterMenu->actionGroup());
    d->m_actionGroupExporter = factory.newActionGroupExporter(d->m_actionGroupMerger->actionGroup(),
                                                        "/com/canonical/indicator/network");

    d->m_busName = factory.newBusName("com.canonical.indicator.network",
                                [](std::string) {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
        std::cout << "acquired" << std::endl;
#endif
                                },
                                [](std::string) {
#ifdef INDICATOR_NETWORK_TRACE_MESSAGES
                                    std::cout << "lost" << std::endl;
#endif
                                });
}

void MenuBuilder::unlockAllModems()
{
    d->m_wwanSection->unlockAllModems();
}

void MenuBuilder::unlockModem(const QString &name)
{
    d->m_wwanSection->unlockModem(name);
}
