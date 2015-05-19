#!/bin/bash
#
# Copyright 2012 Canonical Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author: Juhapekka Piiroinen <juhapekka.piiroinen@canonical.com>
################################################################################

. `dirname $0`/../../build_paths.inc

_CMD=""
_TARGETPATH=$1
_TESTFILEPATH=$2
_MINIMAL=$3

_TARGET=$(basename $1)
_TESTFILE=$(basename $2)
_IMPORT_PATH="${BUILD_DIR}/modules:$QML2_IMPORT_PATH"
_THEMES_PATH="${BUILD_DIR}/modules"
_XML="${BUILD_DIR}/tests/test_$_TARGET_$_TESTFILE.xml"

_ARGS="-p -o -p $_XML,xunitxml -p -o -p -,txt"

set +e

function create_test_cmd {
	if [[ "$_TARGETPATH" = /* ]]; then
      _CMD="dbus-test-runner --task $_TARGETPATH -n $_TESTFILE -m 300"	
	else
      _CMD="dbus-test-runner --task ./$_TARGETPATH -n $_TESTFILE -m 300"
	fi

  if [ "$_MINIMAL" = "minimal" ]; then
      _CMD="$_CMD -p -platform -p minimal"
  fi

  if [ $_TARGETPATH != $_TESTFILEPATH ]; then
      _CMD="$_CMD -p -input -p $_TESTFILEPATH"
  fi
  _CMD="$_CMD -p -maxwarnings -p 40"
}

function execute_test_cmd {
  echo "Executing $_CMD $_ARGS"
  echo "Working directory: $PWD"
  if [ ! -x $_TARGETPATH ]; then
    echo "Error: $_TARGET wasn't built!"
    RESULT=2
  elif [ $DISPLAY ]; then
    # https://bugs.launchpad.net/ubuntu-ui-toolkit/+bug/1256999
    # https://bugreports.qt-project.org/browse/QTBUG-36243
	
    QML2_IMPORT_PATH=${_IMPORT_PATH} UBUNTU_UI_TOOLKIT_THEMES_PATH=${_THEMES_PATH} \
    ALARM_BACKEND=memory SUPPRESS_DEPRECATED_NOTE=yes \
    $_CMD $_ARGS 2>&1 | grep -v 'QFontDatabase: Cannot find font directory'
    RESULT=0
    if [ "x$UITK_TEST_KEEP_RUNNING" != "x1" ]; then
        ${BUILD_DIR}/tests/checkresults.sh $_XML
        RESULT=$*
    fi
  else
    echo "Skipped because no DISPLAY available"
    RESULT=0
  fi
  # segfault
  if [ $RESULT -eq 139 ]; then
    RESULT=2
  fi
  # abort
  if [ $RESULT -eq 134 ]; then
    RESULT=2
  fi
  echo "$_TARGET_$_TESTFILE exited with $RESULT"
  return $RESULT
}

create_test_cmd
execute_test_cmd
RESULT=$?
exit $RESULT
