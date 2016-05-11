import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import QtTest 1.0
import Ubuntu.Test 1.0

Item {
    width: units.gu(40)
    height: units.gu(71)

    MainView {
        anchors.fill: parent
        PageStack {
            id: pageStack
            Page {
                id: page0
                header: PageHeader {
                    title: "0ne"
                    flickable: view
                }

                title: "One"
                visible: false

                ListView {
                    id: view
                    anchors.fill: parent
                    model: 15
                    delegate: Standard {
                        width: ListView.view.width
                        height: units.gu(5)
                        text: index
                    }
                    onTopMarginChanged: print("topMargin = "+topMargin)
                    onContentYChanged: print("contentY = "+contentY)
                    onVisibleChanged: print("visible = "+visible)
                    PullToRefresh {
                        id: pullMe
                        onRefreshingChanged: print("refreshing = "+refreshing)
                        onRefresh: {
                            refreshing = true;
                            refreshing = false;
                            pageStack.push(page1)
                        }
                        onHeightChanged: print("PTR.height = "+height)
                    }
                }
            }
            Page {
                id: page1
                title: "Two"
                visible: false
                Button {
                    anchors.centerIn: parent
                    onClicked: pageStack.pop()
                    text: "back"
                }
            }
            Component.onCompleted: pageStack.push(page0)
        }

//        Timer {
//            id: refreshTimer
//            interval: 800
//            onTriggered: pullMe.refreshing = false;
//        }

//        Timer {
//            id: visibleTimer
//            interval: 1000
//            onTriggered: page0.visible = true
//        }
//        Timer {
//            id: popTimer
//            interval: 1000
//            onTriggered: pageStack.pop()
//        }
    }

    UbuntuTestCase {
        name: "PullToRefreshPageStackPushWhileRefreshing"
        when: windowShown

        function test_push_while_refreshing_bug1578619() {
            var initialTopMargin = view.topMargin;
            var initialContentY = view.contentY;
            flick(view, view.width/2, units.gu(10), 0, units.gu(40));
            wait(1000);
            pageStack.pop();
            tryCompare(view, "moving", false);
            tryCompare(view, "topMargin", initialTopMargin, 500,
                    "Initial topMargin of flickable is not restored after refreshing.");
            tryCompare(view, "contentY", initialContentY, 500,
                    "Initial contentY of flickable is not restored after refreshing.")
        }
    }
}
