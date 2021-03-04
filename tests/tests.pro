TEMPLATE = subdirs
SUBDIRS += unit autopilot

autopilot_module.path = $$[QT_INSTALL_PREFIX]/lib/python3/dist-packages/lomiriuitoolkit
autopilot_module.files = autopilot/lomiriuitoolkit/*

SUBDIRS += api

INSTALLS += autopilot_module

OTHER_FILES += qmlapicheck.sh

check.commands += $$PWD/qmlapicheck.sh || exit 1;
