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

# for debugging: no means just refresh changelog and metainfo with current news file, assumes no template is present (for debugging)
# read -r -e -p "Make a new release , or just refresh changelog and metainfo with current news (y/n) ?: " MAKE_NEW_RELEASE

# assume we want to make a new release every time
MAKE_NEW_RELEASE=y

set -o nounset
set -o noglob
set -o noclobber
#set -o xtrace #Debug

readonly APP_ID='com.github.wwmm.easyeffects'
readonly SCRIPT_DEPS='date dirname realpath xmllint xsltproc sed appstreamcli appstream-util'

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

  if [[ "${old_version}" == "${new_version}" ]]; then
    log_info 'Current app release is already in metainfo. No action taken.'
    log_info 'If you are making a new release, ensure to set a new version in the root meson.build.'
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


check_deps
init "$0"
NEW_RELEASE_VERSION="$(get_version)" || exit 1

check_metainfo_releases "${NEW_RELEASE_VERSION}"

NEW_RELEASE_DATE="$(get_date)" || exit 1



# check if new release version is not already in metainfo.

if [ "$MAKE_NEW_RELEASE" == y ] || [ "$MAKE_NEW_RELEASE" == Y ]
then

    if ! grep -q UNRELEASED_VERSION "${NEWS_FILE}";
    then
        exit_err 'UNRELEASED_VERSION not found in NEWS. Verify NEWS file contains news-release-template at the top.'
    fi
    sed -i "{s/UNRELEASED_VERSION/$NEW_RELEASE_VERSION/g}" "${NEWS_FILE}";
    log_info "Release version $NEW_RELEASE_VERSION put in NEWS file."


    if ! grep -q UNRELEASED_DATE "${NEWS_FILE}";
    then
        exit_err "UNRELEASED_DATE not found in NEWS. Verify NEWS file contains news-release-template at the top."
    fi
    sed -i "{s/UNRELEASED_DATE/$NEW_RELEASE_DATE/g}" "${NEWS_FILE}";
    log_info "Release date $NEW_RELEASE_DATE put in NEWS file."

fi

# adds a date and version number to the latest release in news

# converts a specific type of news file to a markdown changelog file
# since NEWS is not a defined API, it might produce inconsistent results if given a differently formatted news file.
# To avoid problems at the end copy a working template to the top of NEWS for the next release.

log_info "Copying NEWS file to changelog"

cp "${NEWS_FILE}" "${CHANGELOG_FILE}"

log_info "Adjusting changelog formatting"


sed -i 's/~/ /g' "${CHANGELOG_FILE}"

sed -i '/Features:/s/^/### /' "${CHANGELOG_FILE}"
sed -i '/Bugfixes:/s/^/### /' "${CHANGELOG_FILE}"
sed -i '/Notes:/s/^/### /' "${CHANGELOG_FILE}"


sed -i 's/^Version/##/' "${CHANGELOG_FILE}"
sed -i 's/^Released:/###/' "${CHANGELOG_FILE}"

sed -i "1i # Changelog" "${CHANGELOG_FILE}"

log_info "Copying NEWS to metainfo file"
appstreamcli news-to-metainfo --format=text "${NEWS_FILE}" "${METAINFO_FILE}"

log_info "appstreamcli validation"

appstreamcli validate "$METAINFO_FILE"

log_info "appstream-glib validation"

appstream-util validate-relax "$METAINFO_FILE"

if [ "$MAKE_NEW_RELEASE" == y ] || [ "$MAKE_NEW_RELEASE" == Y ]
then
    log_info "Copying template for next release to NEWS"

    sed -i -e "1 e cat $REPO_DIR/util/news-release-template" "${NEWS_FILE}"
fi

