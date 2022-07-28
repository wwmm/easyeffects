#!/usr/bin/env bash

# Update metainfo and changelog using a NEWS.yaml file. Also configures NEWS.yaml file in preparation for the next release.
# After running this script, metainfo and changelog should not need to be modified directly to adjust release notes.

# Release workflow:
# 1. Update version number in meson.build
# 2. Run this script
# 3. Commit, tag, and push to GitHub

# Important: this script is meant to be run at each release. If you do not, it might produce incorrect or confusing results.
# Also, reusing old version numbers for new releases might cause problems.

# some functions reused from https://github.com/wwmm/easyeffects/commit/04bd3e968f7e0ba79b9672882590affe445dc330

# no means just refresh changelog and metainfo with current news.yaml file, will assume template is present representing a future relase. 
# The future release at the top of NEWS.yaml will not be included (since you are only regenerating current releases).

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
  readonly NEWS_FILE="${REPO_DIR}/util/NEWS.yaml"


  if [[ "${BASE_DIR}" != "${REPO_DIR}" ]]; then
    log_info 'Changing current working directory to repo.'
    cd -P -- "${REPO_DIR:?}" || exit 1
  fi
}

log_info() {
  printf "%b" "\e[1m[ \e[36mINFO\e[39m ]\e[0m $1\n"
}

log_err() {
  printf "%b" "\e[1m[ \e[31mERROR\e[39m ]\e[0m $1" >&2
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

  if [ "${old_version}" == "${new_version}" ] && [ "${MAKE_NEW_RELEASE}" == y ]; then
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
  # Works as long as "meson_version:" is defined below "version:"
  file="${file#*version:*\'}"
  version="${file%%\'*}"

  printf "%b" "${version}\n"
}

get_date() {
  local date=''
  
  if ! date="$(date --utc +%F)" ; then
    exit_err 'Failed to get the current date.'
  fi

  printf "%b" "${date}\n"
}

convert_to_changelog() {
  
  # converts a specific type of yaml news file to a markdown changelog file
  # At the end copy a working template to the top of the changelog for the next release.

  log_info "Copying file to changelog"
  TEMP_CHANGELOG_FILE=$(mktemp)

  cp "${TEMP_NEWS}" "${TEMP_CHANGELOG_FILE}"

  log_info "Adjusting changelog formatting"


  sed -i 's/---/ /g' "${TEMP_CHANGELOG_FILE}"

  sed -i 's/- Features/### Features/g' "${TEMP_CHANGELOG_FILE}"
  sed -i 's/- Bug fixes/### Bug fixes/g' "${TEMP_CHANGELOG_FILE}"
  sed -i 's/- Other notes/### Other notes/g' "${TEMP_CHANGELOG_FILE}"

  sed -i 's/Description://g' "${TEMP_CHANGELOG_FILE}"
  
  # replace ratio character with colon
  sed -i 's/∶/:/g' "${TEMP_CHANGELOG_FILE}"


  sed -i 's/^Version:/##/' "${TEMP_CHANGELOG_FILE}"
  sed -i 's/^Date:/###/' "${TEMP_CHANGELOG_FILE}"

  sed -i "1i # Changelog" "${TEMP_CHANGELOG_FILE}"
  
  cp "${TEMP_CHANGELOG_FILE}" "${CHANGELOG_FILE}"

}

remove_unneeded_template() {
  
  # if just refreshing the existing release notes, we don't want the template to be present since it is for the new release

  if [ "${MAKE_NEW_RELEASE}" == n ]
    then
    log_info "Removing template part from temporary NEWS.yaml file, since not generating new release"
    removed_template=false
    for i in {3..1000}
    do
      if [ "$(sed -n "${i}"p "${TEMP_NEWS}")" == "---" ]
      then
        sed -i "1, "$((i - 2))"d" "${TEMP_NEWS}"
        removed_template=true
        break
      fi
    done
    if [ ! "${removed_template}" == true ]
      then
        rm "${TEMP_NEWS:?}"
        log_err "Could not remove the template from your NEWS.yaml file. \n"
        log_err "Please verify it follows the correct format and try again. \n"
        exit_err "Note a NEWS.yaml file template (the top release in the NEWS.yaml file) of up to 1000 lines long is attempted to be removed when regenerating a release. \n"
    fi
  fi

}

prepare_release_entry() {

  # check if new release version is not already in metainfo.
  # adds a date and version number to the latest release in news

  if [ "${MAKE_NEW_RELEASE}" == y ]
  then

    if ! grep -q UNRELEASED_VERSION "${TEMP_NEWS}";
    then
      rm "${TEMP_NEWS:?}"
      exit_err 'UNRELEASED_VERSION not found in NEWS. Verify NEWS file contains news-release-template.yaml at the top.'
    fi
    sed -i "{s/UNRELEASED_VERSION/${NEW_RELEASE_VERSION}/g}" "${TEMP_NEWS}";
    log_info "Release version ${NEW_RELEASE_VERSION} put in NEWS file."


    if ! grep -q UNRELEASED_DATE "${TEMP_NEWS}";
    then
      rm "${TEMP_NEWS:?}"
      exit_err "UNRELEASED_DATE not found in NEWS. Verify NEWS file contains contents of news-release-template.yaml at the top."
    fi
    sed -i "{s/UNRELEASED_DATE/${NEW_RELEASE_DATE}/g}" "${TEMP_NEWS}";
    log_info "Release date ${NEW_RELEASE_DATE} put in NEWS file."

  fi

}

check_appstream_cli() {

  log_info "Checking appstreamcli validate --pedantic"
  
  APPSTREAM_CLI_OUT=$(appstreamcli validate --pedantic --explain "${TEMP_METAINFO_FILE}")
  if [ $? -ne 0 ];
  then
    log_err "appstreamcli validate --pedantic failed \n"
    log_err "appstreamcli: $APPSTREAM_CLI_OUT \n"
    rm "${TEMP_NEWS:?}"
    rm "${TEMP_METAINFO_FILE:?}"
    exit 1
  fi
  log_info "Passed appstreamcli validate --pedantic"

}

check_appstream_util() {

  log_info "Checking appstream-util validate-relax"

  if [ "$(appstream-util validate-relax "${TEMP_METAINFO_FILE}" 2>&1 > /dev/null)" ];
  then
    log_err "appstream-util validate-relax failed \n"
    log_err "appstream-util: $(appstream-util validate-relax "${TEMP_METAINFO_FILE}") \n"
    rm "${TEMP_NEWS:?}"
    rm "${TEMP_METAINFO_FILE:?}"
    exit 1
  fi
  log_info "Passed appstream-util validate-relax"



  log_info "Checking appstream-util validate"
  log_info "This script will hide errors relating to screenshots or style-invalid, as those are considered false positives."
  log_info "However, errors that are not relating to screenshots or style-invalid are considered important to fix." 

  TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT=$(mktemp)
  set +o noclobber
  appstream-util validate "${TEMP_METAINFO_FILE}" > "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}" 2>&1
  set -o noclobber

  sed "/easyeffects-light-screenshot/d" -i "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  sed "/style-invalid/d" -i "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  sed "/failed/d" -i "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  sed "/FAILED/d" -i "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
  sed "/com.github.wwmm.easyeffects.metainfo.xml.in/d" -i "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"

  if [ -s "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}" ]; then
    log_err "appstream-util validate gave non-screenshot related errors \n"
    log_err "appstream-util validate failed \n"
    cat "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT}"
    rm "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT:?}"
    rm "${TEMP_METAINFO_FILE:?}"
    rm "${TEMP_NEWS:?}"
    exit 1
  fi

  log_info "Passed appstream-util validate"

  rm "${TEMP_APPSTREAM_UTIL_EXCESS_OUTPUT:?}"

}

convert_news_to_metainfo() {

  # need to remove URLs as under certain conditions URLs are not permitted in AppStream release notes.
  # only some implementations will actually fail validation an AppStream file on this, though.
  # this only effects the outputted metainfo file, not news or changelog
  
  TEMP_NEWS_CLEANED=$(mktemp)
  cp "${TEMP_NEWS}" "${TEMP_NEWS_CLEANED}"
  
  sed -i 's|https://github.com/wwmm/easyeffects/issues/|issue |g' "${TEMP_NEWS_CLEANED}"
  
  sed -i 's!http[s]\?://\S*!!g' "${TEMP_NEWS_CLEANED}"

  log_info "Converting news to metainfo"

  # must use actual file extension otherwise appstream-util will complain
  TEMP_METAINFO_FILE=$(mktemp --suffix .metainfo.xml.in)

  cp "${METAINFO_FILE}" "${TEMP_METAINFO_FILE}"

  if [ "$(appstreamcli news-to-metainfo --format=yaml "${TEMP_NEWS_CLEANED}" "${TEMP_METAINFO_FILE}" 2>&1 > /dev/null)" ];
  then
    rm "${TEMP_NEWS:?}"
    log_err "Converting news to metainfo failed. \n"
    log_err "Check formatting, don't leave section headers with nothing beneath them. \n"
    log_err "appstreamcli: $(appstreamcli news-to-metainfo --format=yaml "${TEMP_NEWS_CLEANED}" "${TEMP_METAINFO_FILE}" 2>&1 > /dev/null) \n"
    rm "${TEMP_NEWS_CLEANED:?}" 
    rm "${TEMP_METAINFO_FILE:?}"
    exit 1
  fi

  log_info "Succesfully converted news to metainfo"
  
  # replace ratio character with colon
  sed -i 's/∶/:/g' "${TEMP_METAINFO_FILE}"

  rm "${TEMP_NEWS_CLEANED:?}"

}
set +o nounset
if [[ "$1" == "--make-new-release" ]]; then
  MAKE_NEW_RELEASE='y'
elif [[ "$1" == "--no-new-release" ]]; then
  MAKE_NEW_RELEASE='n'
elif [[ ! "$1" == "" ]]; then
  log_err "Unknown argument, exiting \n"
  exit 1
else 
  read -r -e -p "Create a new release? (y/n): " MAKE_NEW_RELEASE
fi
set -o nounset


if ! [ "${MAKE_NEW_RELEASE}" == y ] && ! [ "${MAKE_NEW_RELEASE}" == n ]
then
  exit_err "Invalid input entered. Enter only exactly \"y\" or \"n\" \n"
fi

if [ "${MAKE_NEW_RELEASE}" == n ]
then 
  log_info "Not making a new release, just refreshing changelog and metainfo with current releases."
  log_info "The top entry of the NEWS.yaml file will not be used for refreshing, since that is supposed to be the next release."
fi


check_deps
init "$0"
NEW_RELEASE_VERSION="$(get_version)" || exit 1

check_metainfo_releases "${NEW_RELEASE_VERSION}"

NEW_RELEASE_DATE="$(get_date)" || exit 1

TEMP_NEWS=$(mktemp)
cp "${NEWS_FILE}" "${TEMP_NEWS}"

remove_unneeded_template
prepare_release_entry

convert_news_to_metainfo
check_appstream_cli
check_appstream_util

convert_to_changelog


if [ "${MAKE_NEW_RELEASE}" == y ]
then
  log_info "Copying new dates to NEWS.yaml"
  cp "${TEMP_NEWS}" "${NEWS_FILE}"

  log_info "Copying template for next release to NEWS.yaml"
  sed -i -e "1 e cat ${REPO_DIR}/util/news-release-template.yaml" "${NEWS_FILE}"
fi

log_info "Copying changes to metainfo file"
cp "${TEMP_METAINFO_FILE}" "${METAINFO_FILE}"
rm "${TEMP_METAINFO_FILE:?}"

rm "${TEMP_NEWS:?}"
