/*
 * Copyright 2014-2015 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Lomiri.Components 1.3

MainView {
    width: units.gu(48)
    height: units.gu(60)
    objectName: "mainView"

    Page {
        id: testPage
        objectName: "test_page"
        header: PageHeader {
            objectName: 'test_header'
            title: listView.ViewItems.selectMode ? "In selection mode" : "No action triggered"
        }
        ListView {
            id: listView
            objectName: "test_view"
            anchors.fill: parent
            model: 25
            delegate: ListItem {
                objectName: "listitem" + index
                onPressAndHold: listView.ViewItems.selectMode = true
                leadingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: "delete"
                            objectName: 'delete_action'
                            onTriggered: testPage.title = objectName + " action triggered";
                        },
                        Action {
                            visible: false
                            objectName: 'invisible_action'
                        }
                    ]
                }
                trailingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: "toolkit_input-search"
                            objectName: 'search_action'
                            onTriggered: testPage.title = objectName + " action triggered";
                        },
                        Action {
                            iconName: "edit"
                            objectName: 'edit_action'
                            onTriggered: testPage.title = objectName + " action triggered";
                        },
                        Action {
                            iconName: "email"
                            objectName: 'email_action'
                            onTriggered: testPage.title = objectName + " action triggered";
                        }
                    ]
                }
            }
        }
    }
}
