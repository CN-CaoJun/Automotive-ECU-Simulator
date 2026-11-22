IF(NOT EXISTS "${IMAGE}")
	MESSAGE(FATAL_ERROR "Boot image not found: ${IMAGE}")
ENDIF()

FILE(SIZE "${IMAGE}" IMAGE_SIZE)
IF(IMAGE_SIZE GREATER MAX_SIZE)
	MESSAGE(FATAL_ERROR "Boot image is ${IMAGE_SIZE} bytes, exceeding ${MAX_SIZE} bytes")
ENDIF()

MESSAGE(STATUS "Boot image size: ${IMAGE_SIZE} / ${MAX_SIZE} bytes")
