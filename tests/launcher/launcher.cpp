/*
 * Copyright 2014 Canonical Ltd.
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
 *
 */

#include <iostream>
#include <QtCore/qdebug.h>
#include <QtQuick/QQuickView>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtCore/QFileInfo>
#include <QLibrary>
#include <QOpenGLContext>
#include <QtGui/private/qopenglcontext_p.h>
#include <QtQuick/private/qsgcontext_p.h>

int usage()
{
    QString self(QGuiApplication::instance()->arguments().at(0));
    std::cout << "Usage\n  " << qPrintable(self) << " -testability- frameless -engine FILENAME\n";
    return 1;
}

int main(int argc, const char *argv[])
{
    // QPlatformIntegration::ThreadedOpenGL
    setenv("QML_FORCE_THREADED_RENDERER", "1", 1);
    // QPlatformIntegration::BufferQueueingOpenGL
    setenv("QML_FIXED_ANIMATION_STEP", "1", 1);
    // Oxide and QWebEngine need a shared context
    QScopedPointer<QOpenGLContext> shareContext;
    shareContext.reset(new QOpenGLContext);
    QSGContext::setSharedOpenGLContext(shareContext.data());

    QGuiApplication::setApplicationName("UITK Launcher");
    QGuiApplication application(argc, (char**)argv);
    QStringList args (application.arguments());

    int _testability(args.indexOf("-testability"));
    args.removeAt(_testability);
    int _frameless(args.indexOf("-frameless"));
    args.removeAt(_frameless);
    int _engine(args.indexOf("-engine"));
    args.removeAt(_engine);

    // Testability is only supported out of the box by QApplication not QGuiApplication
    if (_testability > -1 || getenv("QT_LOAD_TESTABILITY")) {
        QLibrary testLib(QLatin1String("qttestability"));
        if (testLib.load()) {
            typedef void (*TasInitialize)(void);
            TasInitialize initFunction = (TasInitialize)testLib.resolve("qt_testability_init");
            if (initFunction) {
                initFunction();
            } else {
                qCritical("Library qttestability resolve failed!");
                return 1;
            }
        } else {
            qCritical("Library qttestability load failed!");
            return 1;
        }
    }

    QQmlEngine engine;
    // The default constructor affects the components tree (autopilot vis)
    QQuickView* view;
    if (_engine > -1) {
        view = new QQuickView();
    } else {
        view = new QQuickView(&engine, NULL);
    }

    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setTitle("UI Toolkit QQuickView");
    if (_frameless > -1) {
        view->setFlags(Qt::FramelessWindowHint);
    }

    // The remaining unnamed argument must be a filename
    if (args.count() == 1) {
        qCritical() << "Missing filename";
        return usage();
    }
    QString filename(args.at(1));
    args.removeAt(0);

    QUrl source(QUrl::fromLocalFile(filename));
    view->setSource(source);
    if (view->errors().count() > 0) {
        return usage();
    }
    view->show();

    if (args.count() > 1) {
        qCritical() << "Invalid arguments passed" << args;
        return usage();
    }

    return application.exec();
}

