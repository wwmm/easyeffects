#!/bin/sh

# https://techbase.kde.org/Development/Tutorials/Localization/i18n_Build_Systems/Outside_KDE_repositories
# the command extractrc is provided by the pacakge kde-dev-scripts

# https://api.kde.org/frameworks/ki18n/html/prg_guide.html

BASEDIR="../"	# root of translatable sources
PROJECT="easyeffects"	# project name
BUGADDR="https://github.com/wwmm/easyeffects/issues"	# MSGID-Bugs
WDIR=`pwd`		# working dir

echo "Preparing rc files"
cd ${BASEDIR}
# we use simple sorting to make sure the lines do not jump around too much from system to system
find src -name '*.rc' -o -name '*.ui' | sort > ${WDIR}/rcfiles.list
xargs --arg-file=${WDIR}/rcfiles.list extractrc > ${WDIR}/rc.cpp
echo "Done preparing rc files"

echo "Preparing source files: C++, C, QML..."
# see above on sorting
find src -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' -o -name '*.qml' \
  | sort > ${WDIR}/infiles.list

echo "rc.cpp" >> ${WDIR}/infiles.list

# preparing the desktop file
echo "Preparing the desktop file"
cp src/contents/com.github.wwmm.easyeffects.desktop.template ${WDIR}
cd ${WDIR}
intltool-extract --quiet --type=gettext/ini com.github.wwmm.easyeffects.desktop.template
echo "com.github.wwmm.easyeffects.desktop.template.h" >> ${WDIR}/infiles.list

# extracting messages on xgettext
cd ${WDIR}
xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 \
	-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 \
  -kN_:1 \
	--msgid-bugs-address="${BUGADDR}" \
	--files-from=infiles.list -D ${BASEDIR} -D ${WDIR} -o ${BASEDIR}/po/${PROJECT}.pot || { echo "error while calling xgettext. aborting."; exit 1; }

echo "Done extracting messages"

echo "Merging translations"

cd ${BASEDIR}/po
catalogs=`find . -name '*.po'`
for catalog in $catalogs; do
  echo $catalog
  msgmerge -o $catalog.new $catalog ${PROJECT}.pot
  mv $catalog.new $catalog
done

echo "Merging news translations"

cd ${BASEDIR}/po_news
catalogs=`find . -name '*.po'`
for catalog in $catalogs; do
  echo $catalog
  msgmerge -o $catalog.new $catalog ${PROJECT}-news.pot
  mv $catalog.new $catalog
done

cd ${WDIR}
intltool-merge --quiet --desktop-style ${BASEDIR}/po com.github.wwmm.easyeffects.desktop.template ${BASEDIR}/src/contents/com.github.wwmm.easyeffects.desktop

echo "Done merging translations"

echo "Cleaning up"

#cp -v infiles.list /tmp

rm rcfiles.list
rm infiles.list
rm rc.cpp
rm com.github.wwmm.easyeffects.desktop.template
rm com.github.wwmm.easyeffects.desktop.template.h

echo "Done"
