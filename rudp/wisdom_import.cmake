set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

# RFM69
list(APPEND sources
	${WISDOM_PROJECT_PATH}/rfm69/src/rfm69_pico.c
)
list(APPEND includes
	${WISDOM_PROJECT_PATH}/rfm69/src/
)
