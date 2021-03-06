set(SNARK_EXTRALIBS)
if(${CURVE} STREQUAL "BN128")
  set(
    SNARK_EXTRALIBS

    ${SNARK_EXTRALIBS}
    ${PROCPS_LIBRARIES}
    zm
  )
endif()

# TODO(madars): Revisit the decision to only offer a static
# library. In theory, it might be nice to offer a shared object, but
# it may be impractical for users, since many of the build-time
# options are mutually exclusive. At least as a static library with a
# simple, reproducible build, users of this library can pick the
# appropriate options themselves for embedding.
add_library(
  snark
  STATIC

  common/data_structures/integer_permutation.cpp
  common/data_structures/set_commitment.cpp
  common/routing_algorithms/as_waksman_routing_algorithm.cpp
  common/routing_algorithms/benes_routing_algorithm.cpp
  gadgetlib1/constraint_profiling.cpp
  gadgetlib2/adapters.cpp
  gadgetlib2/constraint.cpp
  gadgetlib2/gadget.cpp
  gadgetlib2/infrastructure.cpp
  gadgetlib2/integration.cpp
  gadgetlib2/pp.cpp
  gadgetlib2/protoboard.cpp
  gadgetlib2/variable.cpp
)
message("${USE_LINKED_LIBRARIES}")
message(GMP_LIBRARIES=" ${GMP_LIBRARIES}")

  target_link_libraries(
    snark
    
    ff
    ${GMP_LIBRARIES}
    ${GMPXX_LIBRARIES}
    ${CRYPTO_LIBRARIES}
    ${PROCPS_LIBRARIES}
    ${SNARK_EXTRALIBS}
  )
  target_include_directories(
    snark

    PUBLIC
    ${DEPENDS_DIR}/libff
    ${DEPENDS_DIR}/libfqfft
  )

message(CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
install(
  DIRECTORY "" DESTINATION "include/libsnark"
  FILES_MATCHING
  PATTERN "*.hpp"
  PATTERN "*.tcc"
  PATTERN "tests" EXCLUDE
  PATTERN "examples" EXCLUDE
)

install(
  TARGETS
  snark

  DESTINATION lib
)


# Demos
add_executable(
  demo_arithmetization
  EXCLUDE_FROM_ALL

  reductions/ram_to_r1cs/examples/demo_arithmetization.cpp
)
target_link_libraries(
  demo_arithmetization

  snark
  ${Boost_LIBRARIES}
)

add_executable(
  demo_ram_ppzksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark.cpp
)
target_link_libraries(
  demo_ram_ppzksnark

  snark
  ${Boost_LIBRARIES}
)

add_executable(
  demo_ram_ppzksnark_generator
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_generator.cpp
)
target_link_libraries(
  demo_ram_ppzksnark_generator

  snark
  ${Boost_LIBRARIES}
)

add_executable(
  demo_ram_ppzksnark_prover
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_prover.cpp
)
target_link_libraries(
  demo_ram_ppzksnark_prover

  snark
  ${Boost_LIBRARIES}
)

add_executable(
  demo_ram_ppzksnark_verifier
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/examples/demo_ram_ppzksnark_verifier.cpp
)
target_link_libraries(
  demo_ram_ppzksnark_verifier

  snark
  ${Boost_LIBRARIES}
)



# Profiling
add_executable(
  profile_routing_algorithms
  EXCLUDE_FROM_ALL

  common/routing_algorithms/profiling/profile_routing_algorithms.cpp
)
target_link_libraries(
  profile_routing_algorithms

  snark
)

add_executable(
  profile_routing_gadgets
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/routing/profiling/profile_routing_gadgets.cpp
)
target_link_libraries(
  profile_routing_gadgets

  snark
)

add_executable(
  profile_r1cs_ppzksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/r1cs_ppzksnark/profiling/profile_r1cs_ppzksnark.cpp
)
target_link_libraries(
  profile_r1cs_ppzksnark

  snark
)

add_executable(
  profile_r1cs_se_ppzksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/r1cs_se_ppzksnark/profiling/profile_r1cs_se_ppzksnark.cpp
)
target_link_libraries(
  profile_r1cs_se_ppzksnark

  snark
)


add_executable(
  profile_r1cs_gg_ppzksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/profiling/profile_r1cs_gg_ppzksnark.cpp
)
target_link_libraries(
  profile_r1cs_gg_ppzksnark

  snark
)

add_executable(
  profile_ram_ppzksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/profiling/profile_ram_ppzksnark.cpp
)
target_link_libraries(
  profile_ram_ppzksnark

  snark
)



add_executable(
  profile_ram_zksnark
  EXCLUDE_FROM_ALL

  zk_proof_systems/zksnark/ram_zksnark/profiling/profile_ram_zksnark.cpp
)
target_link_libraries(
  profile_ram_zksnark

  snark
  ${Boost_LIBRARIES}
)

# Tests
add_executable(
  common_routing_algorithms_test
  EXCLUDE_FROM_ALL

  common/routing_algorithms/tests/test_routing_algorithms.cpp
)
target_link_libraries(
  common_routing_algorithms_test

  snark
)

add_executable(
  gadgetlib1_simple_test
  EXCLUDE_FROM_ALL

  gadgetlib1/tests/gadgetlib1_test.cpp
)
target_link_libraries(
  gadgetlib1_simple_test

  snark
  gtest_main
)

add_executable(
  gadgetlib1_fooram_test
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/cpu_checkers/fooram/examples/test_fooram.cpp
)
target_link_libraries(
  gadgetlib1_fooram_test

  snark
)

add_executable(
  gadgetlib1_r1cs_ppzksnark_verifier_gadget_test
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/verifiers/tests/test_r1cs_ppzksnark_verifier_gadget.cpp
)
target_link_libraries(
  gadgetlib1_r1cs_ppzksnark_verifier_gadget_test

  snark
)

add_executable(
  gadgetlib2_adapters_test
  EXCLUDE_FROM_ALL

  gadgetlib2/tests/adapters_UTEST.cpp
)
target_link_libraries(
  gadgetlib2_adapters_test

  snark
  gtest_main
)

add_executable(
  gadgetlib2_constraint_test
  EXCLUDE_FROM_ALL

  gadgetlib2/tests/constraint_UTEST.cpp
)
target_link_libraries(
  gadgetlib2_constraint_test

  snark
  gtest_main
)

add_executable(
  gadgetlib2_gadget_test
  EXCLUDE_FROM_ALL

  gadgetlib2/tests/gadget_UTEST.cpp
)
target_link_libraries(
  gadgetlib2_gadget_test

  snark
  gtest_main
)

add_executable(
  gadgetlib2_integration_test
  EXCLUDE_FROM_ALL

  gadgetlib2/examples/simple_example.hpp
  gadgetlib2/tests/integration_UTEST.cpp
  relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.hpp
  relations/constraint_satisfaction_problems/r1cs/examples/r1cs_examples.tcc
  zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.hpp
  zk_proof_systems/ppzksnark/r1cs_ppzksnark/examples/run_r1cs_ppzksnark.h
  gadgetlib2/examples/simple_example.cpp
  gadgetlib2/examples/simple_example.hpp
)
target_link_libraries(
  gadgetlib2_integration_test

  snark
  gtest_main
)

add_executable(
  gadgetlib2_protoboard_test
  EXCLUDE_FROM_ALL

  gadgetlib2/tests/protoboard_UTEST.cpp
)
target_link_libraries(
  gadgetlib2_protoboard_test

  snark
  gtest_main
)

add_executable(
  gadgetlib2_variable_test
  EXCLUDE_FROM_ALL


  gadgetlib2/tests/variable_UTEST.cpp
)
target_link_libraries(
  gadgetlib2_variable_test

  snark
  gtest_main
)

add_executable(
  relations_qap_test
  EXCLUDE_FROM_ALL

  relations/arithmetic_programs/qap/tests/test_qap.cpp
)
target_link_libraries(
  relations_qap_test

  snark
)




add_executable(
  zk_proof_systems_r1cs_se_ppzksnark_test
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/r1cs_se_ppzksnark/tests/test_r1cs_se_ppzksnark.cpp
)
target_link_libraries(
  zk_proof_systems_r1cs_se_ppzksnark_test

  snark
)

add_executable(
  zk_proof_systems_r1cs_gg_ppzksnark_test
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/tests/test_r1cs_gg_ppzksnark.cpp
)
target_link_libraries(
  zk_proof_systems_r1cs_gg_ppzksnark_test

  snark
)

add_executable(
  zk_proof_systems_ram_ppzksnark_test
  EXCLUDE_FROM_ALL

  zk_proof_systems/ppzksnark/ram_ppzksnark/tests/test_ram_ppzksnark.cpp
)
target_link_libraries(
  zk_proof_systems_ram_ppzksnark_test

  snark
)



add_executable(
  test_knapsack_gadget
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/hashes/knapsack/tests/test_knapsack_gadget.cpp
)
target_link_libraries(
  test_knapsack_gadget

  snark
)

add_executable(
  test_merkle_tree_gadgets
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/merkle_tree/tests/test_merkle_tree_gadgets.cpp
)
target_link_libraries(
  test_merkle_tree_gadgets

  snark
)

add_executable(
  test_set_commitment_gadget
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/set_commitment/tests/test_set_commitment_gadget.cpp
)
target_link_libraries(
  test_set_commitment_gadget

  snark
)

add_executable(
  test_sha256_gadget
  EXCLUDE_FROM_ALL

  gadgetlib1/gadgets/hashes/sha256/tests/test_sha256_gadget.cpp
)
target_link_libraries(
  test_sha256_gadget

  snark
)

add_executable(
  ppzk

  zk_proof_systems/ppzksnark/r1cs_ppzksnark/tests/test_ppzk.cpp
)
target_link_libraries(
  ppzk

  snark
)
