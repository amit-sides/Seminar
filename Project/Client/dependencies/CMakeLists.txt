include(FetchContent)

FetchContent_Declare(
  WolfSSL

  GIT_REPOSITORY https://github.com/wolfSSL/wolfssl.git
  GIT_TAG        master)

set(FETCHCONTENT_QUIET NO)
FetchContent_MakeAvailable(WolfSSL)