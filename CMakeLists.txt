# ------------------------------------------------------------------------------
# UniquePtr
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_catch(test_unique unique/test.cpp)

# ------------------------------------------------------------------------------
# SharedPtr + WeakPtr

add_catch(test_shared
    shared/test.cpp)

add_catch(test_weak
    weak/test.cpp
    weak/test_shared.cpp)

add_catch(test_shared_from_this
    shared-from-this/test.cpp
    shared-from-this/test_shared.cpp
    shared-from-this/test_weak.cpp)

target_link_libraries(test_shared allocations_checker)
target_link_libraries(test_weak allocations_checker)
target_link_libraries(test_shared_from_this allocations_checker)

# ------------------------------------------------------------------------------
# IntrusivePtr

add_catch(test_intrusive intrusive/test.cpp)
target_link_libraries(test_intrusive allocations_checker)
