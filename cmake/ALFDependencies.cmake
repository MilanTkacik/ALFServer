find_package(Boost 1.56
  COMPONENTS
  unit_test_framework
  filesystem
  system
  program_options
  ${boost_python_component}
  REQUIRED
  )
find_package(Git QUIET)
find_package(Common REQUIRED)
find_package(InfoLogger REQUIRED)
find_package(ReadoutCard REQUIRED)
find_package(DIM REQUIRED)

o2_define_bucket(
  NAME
  o2_alf_bucket

  DEPENDENCIES
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  pthread
  ${Common_LIBRARIES}
  ${InfoLogger_LIBRARIES}
  ${ReadoutCard_LIBRARIES}
  ${DIM_LIBRARY}

  SYSTEMINCLUDE_DIRECTORIES
  ${Boost_INCLUDE_DIR}
  ${Common_INCLUDE_DIRS}
  ${InfoLogger_INCLUDE_DIRS}
  ${ReadoutCard_INCLUDE_DIRS}
  ${DIM_INCLUDE_DIRS}
)
