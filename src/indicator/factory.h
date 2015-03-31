/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *     Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <menu-builder.h>
#include <indicator-menu.h>
#include <root-state.h>
#include <connectivity-service/connectivity-service.h>
#include <menumodel-cpp/menu-exporter.h>
#include <menumodel-cpp/action-group-merger.h>
#include <sections/quick-access-section.h>
#include <sections/wifi-section.h>
#include <sections/wwan-section.h>

#include <memory>

class Factory
{
    struct Private;
    std::shared_ptr<Private> d;

public:
    Factory();

    virtual ~Factory();

    virtual std::unique_ptr<MenuBuilder> newMenuBuilder();

    virtual std::unique_ptr<connectivity_service::ConnectivityService> newConnectivityService();

    virtual std::unique_ptr<RootState> newRootState();

    virtual std::unique_ptr<IndicatorMenu> newIndicatorMenu(RootState::Ptr rootState, const std::string &prefix);

    virtual std::unique_ptr<MenuExporter> newMenuExporter(const std::string &path, MenuModel::Ptr menuModel);

    virtual std::unique_ptr<QuickAccessSection> newQuickAccessSection(SwitchItem::Ptr wifiSwitch);

    virtual std::unique_ptr<WwanSection> newWwanSection();

    virtual std::unique_ptr<WifiSection> newWiFiSection();

    virtual ActionGroupMerger::UPtr newActionGroupMerger();

    virtual ActionGroupExporter::UPtr newActionGroupExporter(ActionGroup::Ptr actionGroup, const std::string &path);

    virtual BusName::UPtr newBusName(std::string name,
                                     std::function<void(std::string)> acquired,
                                     std::function<void(std::string)> lost);
};
