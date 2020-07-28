# SPDX-License-Identifier: BSD-3-Clause

add_local_sources(sof sof_dsm.c sof_dsm_inf.c)

target_include_directories(sof PUBLIC ${PROJECT_SOURCE_DIR}/src/audio/dsm/dsm_api/inc)