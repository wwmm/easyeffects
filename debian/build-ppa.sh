#!/bin/bash
# I use this script to build and publish deb packages in ppa:mikhailnov/pulseeffects (https://launchpad.net/~mikhailnov/+archive/ubuntu/pulseeffects)
# I publish it to allow other people to use it and make it possible to maintain a new PPA easily in case I stop doing it for some reason
# I think, it can also be used for maintaining packages in mainline Debian (minor modifications required)

pkg_name="pulseeffects"

# this allows the script to be ran both from the root of the source tree and from ./debian directory
dir_start="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [ "$(basename "${dir_start}")" = 'debian' ]; then
	cd ..
fi

debian/rules clean
dir0="$(pwd)"
old_header=$(head -1 ./debian/changelog)
old_format_source=$(cat ./debian/source/format)

for i in bionic cosmic
do
	old_version="$(cat ./debian/changelog | head -n 1 | awk -F "(" '{print $2}' | awk -F ")" '{print $1}')"
	new_version="${old_version}~${i}1"
	sed -i -re "s/${old_version}/${new_version}/g" ./debian/changelog
	sed -i -re "1s/unstable/$i/" ./debian/changelog
	rm -fv ./debian/source/format
	# -I to exclude .git; -d to allow building .changes file without build dependencies installed
	dpkg-buildpackage -I -S -sa -d
	sed  -i -re "1s/.*/${old_header}/" ./debian/changelog
	echo "$old_format_source" >./debian/source/format
	cd ..
	
	# change PPA names to yours, you may leave only one PPA; I upload hw-probe to 2 different PPAs at the same time
	for ppa_name in ppa:mikhailnov/pulseeffects ppa:mikhailnov/utils ppa:mikhailnov/desktop1-dev
	do
		dput -f "$ppa_name" "$(/bin/ls -tr ${pkg_name}_*_source.changes | tail -n 1)"
	done
	
	cd "$dir0"
	sleep 1
done

debian/rules clean
cd "$dir_start"
