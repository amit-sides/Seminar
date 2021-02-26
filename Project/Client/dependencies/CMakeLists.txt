include(FetchContent)

FetchContent_Declare(
  mbedTLS

  GIT_REPOSITORY https://github.com/ARMmbed/mbedtls.git
  GIT_TAG        development)

FetchContent_MakeAvailable(mbedTLS)
FetchContent_GetProperties(mbedTLS)
message(Include: ${mbedtls_SOURCE_DIR}/include)
include_directories("${mbedtls_SOURCE_DIR}/include")