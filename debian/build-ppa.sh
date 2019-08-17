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
tmp_suffix="buildPPA_$(date +%s)_tmp"

for i in bionic disco eoan
do
	old_version="$(cat ./debian/changelog | head -n 1 | awk -F "(" '{print $2}' | awk -F ")" '{print $1}')"
	new_version="${old_version}~${i}1"
	real_version="$(echo "$old_version" | cut -d'-' -f1)"
	sed -i -re "s/${old_version}/${new_version}/g" ./debian/changelog
	sed -i -re "1s/unstable/$i/" ./debian/changelog
	if grep -q '(git)' ./debian/source/format; then
		GIT=1
		old_source_format="$(cat ./debian/source/format)"
		# TODO: Launchpad PPA does not accept in '3.0 (git)' for some reasons
		rm -fv ./debian/source/format
		git add .
		# dpkg-buildpackage also tries to diff current git vs tarball if it finds it in ../
		while read line
		do
			# TODO: check if target file already exists
			mv -v "$line" "${line}.${tmp_suffix}"
		done < <(find ../ -maxdepth 1 -name "${pkg_name}_${real_version}.orig.tar*")
	fi
	# -I to exclude .git; -d to allow building .changes file without build dependencies installed
	dpkg-buildpackage -I -S -sa -d
	sed  -i -re "1s/.*/${old_header}/" ./debian/changelog
	[ "$GIT" = 1 ] && git reset
	[ -n "$old_source_format" ] && echo "$old_source_format" > ./debian/source/format

	# dpkg-buildpackage also tries to diff current git vs tarball if it finds it in ../
	while read line
	do
		new_name="$(echo "$line" | sed -e "s/.${tmp_suffix}$//g")"
		# TODO: check if target file already exists
		mv -v "$line" "$new_name"
	done < <(find ../ -maxdepth 1 -name "*.${tmp_suffix}")

	cd ..

	# change PPA names to yours, you may leave only one PPA; I upload hw-probe to 2 different PPAs at the same time
	for ppa_name in ppa:mikhailnov/pulseeffects ppa:mikhailnov/utils ppa:mikhailnov/desktop1-dev
	do
		# example file name: pulseeffects_4.4.3-1~bionic1_source.changes
		if [ -f "${pkg_name}_${new_version}_source.changes" ]
			then dput -f "$ppa_name" "${pkg_name}_${new_version}_source.changes"
			else echo ".changes file ${pkg_name}_${new_version}_source.changes not found, not uploading anything!"
		fi
	done

	cd "$dir0"
	sleep 1
done

debian/rules clean
cd "$dir_start"
