add_subdirectory(scm) 

include_directories(${BOOST_INCLUDEDIR})

add_executable(mcmc_sampler mcmc_sampler.cpp)
# add_executable(rejection_sampler rejection_sampler.cpp)

target_link_libraries (mcmc_sampler scm)
# target_link_libraries (rejection_sampler scm)

target_link_libraries(mcmc_sampler ${Boost_LIBRARIES})
# target_link_libraries(rejection_sampler ${Boost_LIBRARIES})
