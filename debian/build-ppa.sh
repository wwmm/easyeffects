#!/bin/bash
# I use this script to build and publish deb packages in ppa:mikhailnov/pulseeffects (https://launchpad.net/~mikhailnov/+archive/ubuntu/pulseeffects)
# I publish it to allow other people to use it and make it possible to maintain a new PPA easily in case I stop doing it for some reason

dir0="$(pwd)"
old_header=$(head -1 ./changelog)

for i in artful bionic
do
	old_version="$(cat ./changelog | head -n 1 | awk -F "(" '{print $2}' | awk -F ")" '{print $1}')"
	new_version="${old_version}~${i}1"
	sed -i -re "s/${old_version}/${new_version}/g" ./changelog
	sed -i -re "1s/unstable/$i/" ./changelog
	cd ..
	dpkg-buildpackage -S -sa
	sed  -i -re "1s/.*/${old_header}/" ./debian/changelog
	cd ..
	dput -f ppa:mikhailnov/pulseeffects "$(ls -tr pulseeffects_*_source.changes | tail -n 1)" || exit 1
	cd "${dir0}"
	sleep 1
done

cd ..
