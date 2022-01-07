#!/usr/bin/env bash

# Update metainfo and changelog using a NEWS file. Also configures NEWS file in preparation for the next release.
# After running update-release-files.sh, metainfo and changelog should not need to be modified directly to adjust release notes.

# Release workflow:
# 1. Update version number in meson.build
# 2. Run this script
# 3. Commit, tag, and push to GitHub

# Important: this script is meant to be run at each release. If you do not, it might produce incorrect or confusing results.
# Also, reusing old version numbers for new releases might cause problems.

# some functions reused from https://github.com/wwmm/easyeffects/commit/04bd3e968f7e0ba79b9672882590affe445dc330

# for debugging: no means just refresh changelog and metainfo with current news file, will assume template is present representing a future relase. 
# The future release at the top of NEWS will not be included (since you are only regenerating current releases).
read -r -e -p "Create a new release? (y/n): " MAKE_NEW_RELEASE

# assume we want to make a new release every time
# MAKE_NEW_RELEASE=y

set -o nounset
set -o noglob
set -o noclobber
#set -o xtrace #Debug

readonly APP_ID='com.github.wwmm.easyeffects'
readonly SCRIPT_DEPS='date dirname realpath xmllint xsltproc sed appstreamcli appstream-util mktemp'

BASE_DIR='.'
CMD_DIR=''
REPO_DIR=''
METAINFO_FILE=''

# Configurable
DATA_DIR='data'

init() {
  local cmd="$1"
  
  readonly BASE_DIR="$(pwd -P)"
  readonly CMD_DIR="$(dirname "${cmd}")"
  readonly REPO_DIR="$(realpath "${CMD_DIR}"/..)"
  readonly DATA_DIR="${REPO_DIR}/${DATA_DIR}"
  readonly METAINFO_FILE="${DATA_DIR}/${APP_ID}.metainfo.xml.in"
  readonly CHANGELOG_FILE="${REPO_DIR}/CHANGELOG.md"
  readonly NEWS_FILE="${REPO_DIR}/util/NEWS"


  if [[ "${BASE_DIR}" != "${REPO_DIR}" ]]; then
    log_info 'Changing current working directory to repo.'
    cd -P -- "${REPO_DIR}" || exit 1
  fi
}

log_info() {
  printf "\e[1m[ \e[36mINFO\e[39m ]\e[0m $1\n"
}

log_err() {
  printf "\e[1m[ \e[31mERROR\e[39m ]\e[0m $1" >&2
}

exit_err() {
  log_err "$1"
  exit 1
}

check_deps() {
  local missing_deps=''

  for dep in ${SCRIPT_DEPS}; do
    if ! which "${dep}" > /dev/null 2>&1; then
      missing_deps="${missing_deps} ${dep}"
    fi
  done

  if [[ -n "${missing_deps}" ]]; then
    exit_err "Missing commands:${missing_deps}\n"
  fi
}

check_metainfo_releases() {
  local new_version="$1"
  local old_version=''
  local xpath='string(//release[1]/@version)'

  old_version="$(xmllint --xpath "${xpath}" "${METAINFO_FILE}")"
  if [[ "$?" -ne 0 ]]; then
    exit_err "Failed to find any existing releases in ${METAINFO_FILE}."
  fi

  if [ "${old_version}" == "${new_version}" ] && [ "$MAKE_NEW_RELEASE" == y ]; then
    log_info 'Current app release is already in metainfo. No action taken.'
    log_info 'Since you said you are making a new release, ensure to set a new version in the root meson.build.'
    exit 0
  fi
}

get_version() {
  local file=''
  local version=''

  # Read main project meson.build file
  file="${REPO_DIR}/$(< meson.build)"
  if [[ "$?" -ne 0 ]]; then
    exit_err 'Failed to read meson.build file.'
  fi
  # Extract version string
  # Works as long as "meson_version:" is defined bellow "version:"
  file="${file#*version:*\'}"
  version="${file%%\'*}"

  printf "${version}\n"
}

get_date() {
  local date=''

  date="$(date --utc +%F)"
  if [[ "$?" -ne 0 ]]; then
    exit_err 'Failed to get the current date.'
  fi

  printf "${date}\n"
}

# TODO add linter that checks if no more than one empty line is along
# e.g.
# 
# 
# stuff 
# is bad and makes news-to-metainfo look weird (it writes This reelease fixes the following bugs, then puts Bugfixes:)

if ! [ "$MAKE_NEW_RELEASE" == y ] && ! [ "$MAKE_NEW_RELEASE" == n ]
then
  exit_err "Invalid input entered. Enter only exactly \"y\" or \"n\" \n"
fi

if [ "$MAKE_NEW_RELEASE" == n ]
then 
  log_info "Not making a new release, just refreshing changelog and metainfo with current releases."
  log_info "The top entry of the NEWS file will not be used for refreshing, since that is supposed to be the next release."
fi


check_deps
init "$0"
NEW_RELEASE_VERSION="$(get_version)" || exit 1

check_metainfo_releases "${NEW_RELEASE_VERSION}"

NEW_RELEASE_DATE="$(get_date)" || exit 1


# a temp news file we will use throughout
# this is the one our fun changes go into. 
# The only case we actually edit the original is one making a new release and everthing is succesful.

TEMP_NEWS=$(mktemp)
cp "${NEWS_FILE}" "${TEMP_NEWS}"

# if we're not going to keep the changes to news anyhow, we can use this as part of the refresh
# remove the template part from news, then make temp news, then copy temp news to changelog and metainfo

if [ "$MAKE_NEW_RELEASE" == n ]
  then
  log_info "Removing template part from temporary NEWS file, since not generating new release"
  REMOVED_TEMPLATE=false
  for i in {3..1000}
  do
    if [ "$(sed -n "${i}"p "$TEMP_NEWS")" == "~~~~~~~~~~~~~~" ]
    then
      sed -i "1, "$((i - 2))"d" "${TEMP_NEWS}"
      REMOVED_TEMPLATE=true
      break
    fi
  done
  if [ ! $REMOVED_TEMPLATE == true ]
    then
      rm "${TEMP_NEWS}"
      log_err "Could not remove the template from your NEWS file. \n"
      log_err "Please verify it follows the correct format and try again. \n"
      exit_err "Note a NEWS file template (the top release in a news file) of up to 1000 lines long is attempted to be removed when regenerating a release. \n"
  fi
fi




# avoids AppStream complaints, and wasted space
log_info "Checking if empty list lines found in NEWS file"

if grep -q "\- $" "${TEMP_NEWS}";
then
  rm "${TEMP_NEWS}"
  exit_err "Unused list line \"- \" found in NEWS. Remove all empty unused list lines and try again. \n"
fi

log_info "No empty list lines found in NEWS file"



# check if new release version is not already in metainfo.
# adds a date and version number to the latest release in news

if [ "$MAKE_NEW_RELEASE" == y ]
then

  if ! grep -q UNRELEASED_VERSION "${TEMP_NEWS}";
  then
    rm "${TEMP_NEWS}"
    exit_err 'UNRELEASED_VERSION not found in NEWS. Verify NEWS file contains news-release-template at the top.'
  fi
  sed -i "{s/UNRELEASED_VERSION/$NEW_RELEASE_VERSION/g}" "${TEMP_NEWS}";
  log_info "Release version $NEW_RELEASE_VERSION put in NEWS file."


  if ! grep -q UNRELEASED_DATE "${TEMP_NEWS}";
  then
    rm "${TEMP_NEWS}"
    exit_err "UNRELEASED_DATE not found in NEWS. Verify NEWS file contains news-release-template at the top."
  fi
  sed -i "{s/UNRELEASED_DATE/$NEW_RELEASE_DATE/g}" "${TEMP_NEWS}";
  log_info "Release date $NEW_RELEASE_DATE put in NEWS file."

fi






# need to remove URLs as under certain conditions URLs are not permitted in AppStream release notes.
# only some implementations will actually fail validation an AppStream file on this, though.
# this only effects the outputted metainfo file, not news or changelog

TEMP_NEWS_URL_REMOVAL=$(mktemp)
cp "${TEMP_NEWS}" "${TEMP_NEWS_URL_REMOVAL}"

sed -i 's!http[s]\?://\S*!!g' "${TEMP_NEWS_URL_REMOVAL}"

log_info "Converting news to metainfo"

# must use actual file extension because appstream-util will complain
TEMP_METAINFO_FILE="${REPO_DIR}/util/${APP_ID}.metainfo.xml.in"
cp "${METAINFO_FILE}" "${TEMP_METAINFO_FILE}"

if [ "$(appstreamcli news-to-metainfo --format=text "${TEMP_NEWS_URL_REMOVAL}" "${TEMP_METAINFO_FILE}" 2>&1 > /dev/null)" ];
then
  rm "${TEMP_NEWS}"
  log_err "Converting news to metainfo failed. \n"
  log_err "Check formatting, don't leave section headers with nothing beneath them. \n"
  log_err "appstreamcli: $(appstreamcli news-to-metainfo --format=text "${TEMP_NEWS_URL_REMOVAL}" "${TEMP_METAINFO_FILE}" 2>&1 > /dev/null) \n"
  rm "$TEMP_NEWS_URL_REMOVAL" 
  rm "$TEMP_METAINFO_FILE"
  exit 1
fi

log_info "Succesfully converted news to metainfo"

rm "${TEMP_NEWS_URL_REMOVAL}"



log_info "Checking appstreamcli validate --pedantic"

if [ "$(appstreamcli validate --pedantic "$TEMP_METAINFO_FILE" 2>&1 > /dev/null)" ];
then
  log_err "appstreamcli validate --pedantic failed \n"
  log_err "appstreamcli: $(appstreamcli validate --pedantic "$TEMP_METAINFO_FILE") \n"
  rm "${TEMP_NEWS}"
  rm "${TEMP_METAINFO_FILE}"
  exit 1
fi
log_info "appstreamcli: $(appstreamcli validate --pedantic "$TEMP_METAINFO_FILE")"
log_info "Passed appstreamcli validate --pedantic"



log_info "Checking appstream-util validate-relax"

if [ "$(appstream-util validate-relax "$TEMP_METAINFO_FILE" 2>&1 > /dev/null)" ];
then
  log_err "appstream-util validate-relax failed \n"
  log_err "appstream-util: $(appstream-util validate-relax "$TEMP_METAINFO_FILE") \n"
  rm "${TEMP_NEWS}"
  rm "${TEMP_METAINFO_FILE}"
  exit 1
fi
log_info "Passed appstream-util validate-relax"



log_info "Checking appstream-util validate"
log_info "This script will hide errors relating to screenshots, as those are considered false positives."
log_info "However, errors that are not relating to screenshots are considered important to fix." 

TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT=$(mktemp)
set +o noclobber
appstream-util validate "${TEMP_METAINFO_FILE}" > "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}" 2>&1
set -o noclobber

# remove some screenshot complaints

sed "/easyeffects-light-screenshot/d" -i "$TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT"
sed "/failed/d" -i "$TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT"
sed "/FAILED/d" -i "$TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT"
sed "/com.github.wwmm.easyeffects.metainfo.xml.in/d" -i "$TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT"


if [ -s "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}" ]; then
  log_err "appstream-util validate gave non-screenshot related errors \n"
  log_err "appstream-util validate failed \n"
  cat "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  rm "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  rm "${TEMP_METAINFO_FILE}"
  rm "${TEMP_NEWS}"
  exit 1
fi

log_info "Passed appstream-util validate"

rm "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"


# converts a specific type of news file to a markdown changelog file
# since NEWS is not a defined API, it might produce inconsistent results if given a differently formatted news file.
# To avoid problems at the end copy a working template to the top of NEWS for the next release.

log_info "Copying NEWS file to changelog"

cp "${TEMP_NEWS}" "${CHANGELOG_FILE}"

log_info "Adjusting changelog formatting"


sed -i 's/~/ /g' "${CHANGELOG_FILE}"

sed -i '/Features:/s/^/### /' "${CHANGELOG_FILE}"
sed -i '/Bugfixes:/s/^/### /' "${CHANGELOG_FILE}"
sed -i '/Notes:/s/^/### /' "${CHANGELOG_FILE}"


sed -i 's/^Version/##/' "${CHANGELOG_FILE}"
sed -i 's/^Released:/###/' "${CHANGELOG_FILE}"

sed -i "1i # Changelog" "${CHANGELOG_FILE}"


if [ "$MAKE_NEW_RELEASE" == y ]
then
  log_info "Copying in new dates to NEWS"
  cp "${TEMP_NEWS}" "${NEWS_FILE}"

  log_info "Copying template for next release to NEWS"
  sed -i -e "1 e cat $REPO_DIR/util/news-release-template" "${NEWS_FILE}"
fi

log_info "Copying in new metainfo file"
cp "${TEMP_METAINFO_FILE}" "${METAINFO_FILE}"
rm "${TEMP_METAINFO_FILE}"

rm "${TEMP_NEWS}"
