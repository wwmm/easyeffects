#!/usr/bin/env bash

set -o nounset
set -o noglob
set -o noclobber
#set -o xtrace #Debug

# -----------------------------------------------------------------------------
#                                  Constants
# -----------------------------------------------------------------------------

readonly APP_ID='com.github.wwmm.pulseeffects'
readonly SCRIPT_DEPS='date dirname realpath xmllint xsltproc'

BASE_DIR='.'
CMD_DIR=''
REPO_DIR=''
APPDATA_FILE=''

# Configurable
DATA_DIR='data'

# -----------------------------------------------------------------------------
#                                  Functions
# -----------------------------------------------------------------------------

init() {
  local cmd="$1"
  
  readonly BASE_DIR="$(pwd -P)"
  readonly CMD_DIR="$(dirname "${cmd}")"
  readonly REPO_DIR="$(realpath "${CMD_DIR}"/..)"
  readonly DATA_DIR="${REPO_DIR}/${DATA_DIR}"
  readonly APPDATA_FILE="${DATA_DIR}/${APP_ID}.appdata.xml.in"

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

check_appdata_releases() {
  local new_version="$1"
  local old_version=''
  local xpath='string(//release[1]/@version)'

  old_version="$(xmllint --xpath "${xpath}" "${APPDATA_FILE}")"
  if [[ "$?" -ne 0 ]]; then
    exit_err "Failed to find any existing releases in ${APPDATA_FILE}."
  fi

  if [[ "${old_version}" == "${new_version}" ]]; then
    log_info 'Current app release is already in appdata. No action taken.'
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

add_new_release() {
  local xsl_file="${CMD_DIR}/add-appdata-release.xsl"
  local version=''
  local date=''

  version="$(get_version)" || exit 1
  check_appdata_releases "${version}"
  date="$(get_date)" || exit 1

  log_info "Adding release information for ${version} to appdata file."
  xsltproc \
    --stringparam version "${version}" \
    --stringparam date "${date}" \
    -o "${APPDATA_FILE}" "${xsl_file}" "${APPDATA_FILE}"
  if [[ "$?" -ne 0 ]]; then
    exit_err "Failed to apply changes to ${APPDATA_FILE}."
  fi
}

main() {
  check_deps
  init "$0"
  add_new_release
}

main
