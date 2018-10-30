cmake_minimum_required(VERSION 3.0.0)
project(KSquareUtilities VERSION 0.1.0)

enable_testing()

subdirs(KKBase)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
