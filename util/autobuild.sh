#!/usr/bin/env bash
# This ugly script helps to build a new version of PulseEffects
# Currently for DEB package only
# Author: Mikhail Novosyolov <mikhailnov@dumalogiya.ru>

pkg_name="pulseeffects"
git_upstream_url="https://github.com/wwmm/pulseeffects.git"
stdate="$(date +%s)"
day_name="$(env LANG=c date --date="@${stdate}" +%a)"
month_name="$(env LANG=c date --date="@${stdate}" +%b)"
year="$(env LANG=c date --date="@${stdate}" +%Y)"
day_month="$(env LANG=c date --date="@${stdate}" +%d)"
time="$(env LANG=c date --date="@${stdate}" +%H:%m:%S)"
timezone="$(env LANG=c date --date="@${stdate}" +%z)"

dir_start="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ "$(basename "${dir_start}")" = 'util' ]; then
	cd ..
fi
dir0="$(pwd)"

echo_help(){
	echo "Usage: nv|nw|new_version, ppa, lc|local_test, check_fuzzy|check_fuzzy_po|cf, full"
}

git_sync_upstream(){
	if ! git remote -v | grep -q "$git_upstream_url"; then
		git remote add upstream "$git_upstream_url"
	fi
	
	git fetch upstream
	
	# git merge returns 0 only if merge was successful
	if git merge upstream/master
		then
			:
		else
			echo ""
			echo "Merge conflicts! Cannot continue!"
			echo ""
			exit 1
	fi
}

debian_changelog_new_entry(){
	if [ ! -f debian/changelog.tpl ]; then
		echo "No debian/changelog.tpl, cannot continue!"
		exit 1
	fi
	if [ -z "$new_version" ]; then echo "new_version is empty"; exit 1; fi
	cat debian/changelog.tpl | \
		sed "s/%new_version%/${new_version}/g" | \
		sed "s/%day_name%/${day_name}/g" | \
		sed "s/%day_month%/${day_month}/g" | \
		sed "s/%month_name%/${month_name}/g" | \
		sed "s/%year%/${year}/g" | \
		sed "s/%time%/${time}/g" | \
		sed "s/%timezone%/${timezone}/g" | \
		tee -a "${dir0}/debian/changelog.new"
	mv debian/changelog debian/changelog.old
	echo " " >> debian/changelog.new
	cat debian/changelog.new debian/changelog.old > debian/changelog
	rm -f debian/changelog.new debian/changelog.old
}

check_fuzzy_po(){
	for lang in ru
	do
		if grep -q "^#, fuzzy" "po/${lang}.po"
			then
				read -p "Fuzzies FOUND in localization ${lang}"
			else
				echo "No fuzzies found in localization ${lang}"
		fi
		
		if grep -q "^#, fuzzy" "help/${lang}/${lang}.po"
			then
				read -p "Fuzzies FOUND in help ${lang}"
				echo "No fuzzies found in help ${lang}"
			else
				echo "No fuzzies found in help ${lang}"
		fi
	done
}

new_version(){
	# env USCAN=0 util/autobuild.sh
	if [ ! "$USCAN" = '0' ]
		then
			if env LANG=c uscan | grep -qi 'Newer package available'; then
				new_version="$(env LANG=c uscan --no-download | grep 'Newest version of' | awk -F ', ' '{print $1}' | awk -F ' ' '{print $NF}')"
			fi
		else
			:
	fi
	git_sync_upstream
	git commit -m "Updated to ${new_version} (autobuild)" debian/changelog
	debian_changelog_new_entry
	check_fuzzy_po
}

ppa(){
	pushd debian
	./build-ppa.sh
	popd
}

git_push(){
	git push
}

local_test(){
	dpkg-buildpackage
	last_version="$(head -n 1 debian/changelog | tr -d "()" | awk -F ' ' '{print $2}')"
	sudo apt install ../*${pkg_name}*${last_version}*.deb -y && \
	if pulseeffects
		then return 0
		else return 1
	fi
	debian/rules clean
}

case "$1" in
	nv|nw|new_version )
		new_version
	;;
	ppa )
		ppa
	;;
	full )
		new_version
		if local_test; then
			ppa
			git_push
		fi
	;;
	lc|local_test )
		#new_version
		local_test
	;;
	check_fuzzy|check_fuzzy_po|cf )
		check_fuzzy_po
	;;
	* )
		echo "Current dir is: $(pwd) ."
		echo_help
	;;
esac
