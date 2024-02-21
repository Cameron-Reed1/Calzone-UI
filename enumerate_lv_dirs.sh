#!/usr/bin/env bash


LVGL_PATH="deps/lvgl"
OUT_FILE="lv_dirs.mk"

printf "LVGL_PATH := ${LVGL_PATH}\n\n" > "${OUT_FILE}"

printf "LVGL_DIRS := " >> "${OUT_FILE}"
find "${LVGL_PATH}/src" -type d | tr "\n" " " >> "${OUT_FILE}"

