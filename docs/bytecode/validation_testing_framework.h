/**
 * @file validation_testing_framework.h
 * @brief Comprehensive Validation and Testing Framework for AST-Aware System
 * 
 * This module implements a systematic testing methodology that validates
 * functional correctness, semantic preservation, and performance characteristics
 * across all phases of the AST-Aware compilation pipeline.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_VALIDATION_TESTING_FRAMEWORK_H
#define OBINEXUS_VALIDATION_TESTING_FRAMEWORK_H

#include "ast_aware_system.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Test case types for systematic validation
 */
typedef enum {
    OBINEXUS_TEST_UNIT,                 /**< Unit test for individual components */
    OBINEXUS_TEST_INTEGRATION,          /**< Integration test between components */
    OBINEXUS_TEST_SYSTEM,               /**< System-level end-to-end test */
    OBINEXUS_TEST_REGRESSION,           /**< Regression test against known configurations */
    OBINEXUS_TEST_PERFORMANCE,          /**< Performance characteristic validation */
    OBINEXUS_TEST_SEMANTIC_EQUIVALENCE, /**< Semantic preservation validation */
    OBINEXUS_TEST_CROSS_PLATFORM,       /**< Cross-platform consistency validation */
    OBINEXUS_TEST_STRESS,               /**< Stress testing under resource constraints */
    OBINEXUS_TEST_SECURITY              /**< Security model validation */
} obinexus_test_type_t;

/**
 * @brief Test case severity levels
 */
typedef enum {
    OBINEXUS_TEST_SEVERITY_CRITICAL,    /**< Critical functionality - system cannot operate without */
    OBINEXUS_TEST_SEVERITY_HIGH,        /**< High importance - significant functionality impact */
    OBINEXUS_TEST_SEVERITY_MEDIUM,      /**< Medium importance - moderate functionality impact */
    OBINEXUS_TEST_SEVERITY_LOW,         /**< Low importance - minimal functionality impact */
    OBINEXUS_TEST_SEVERITY_INFO         /**< Informational - documentation or metrics only */
} obinexus_test_severity_t;

/**
 * @brief Test execution result
 */
typedef enum {
    OBINEXUS_TEST_RESULT_PASS,          /**< Test passed successfully */
    OBINEXUS_TEST_RESULT_FAIL,          /**< Test failed */
    OBINEXUS_TEST_RESULT_SKIP,          /**< Test skipped due to conditions */
    OBINEXUS_TEST_RESULT_ERROR,         /**< Test encountered execution error */
    OBINEXUS_TEST_RESULT_TIMEOUT        /**< Test timed out */
} obinexus_test_result_t;

/**
 * @brief Individual test case definition
 */
typedef struct obinexus_test_case {
    /* Test identification */
    char *test_id;                      /**< Unique test identifier */
    char *test_name;                    /**< Human-readable test name */
    char *test_description;             /**< Detailed test description */
    
    /* Test classification */
    obinexus_test_type_t test_type;     /**< Type of test */
    obinexus_test_severity_t severity;  /**< Test severity level */
    char **test_tags;                   /**< Array of test tags for categorization */
    size_t num_tags;                    /**< Number of test tags */
    
    /* Test execution */
    bool (*setup_function)(void);       /**< Test setup function */
    bool (*test_function)(void);        /**< Main test execution function */
    void (*teardown_function)(void);    /**< Test cleanup function */
    
    /* Test data */
    void *test_input_data;              /**< Input data for test */
    void *expected_output_data;         /**< Expected output data */
    void *actual_output_data;           /**< Actual output data (populated during execution) */
    
    /* Execution constraints */
    uint32_t timeout_seconds;           /**< Maximum execution time */
    size_t max_memory_usage_mb;         /**< Maximum memory usage */
    
    /* Dependencies and prerequisites */
    char **required_tests;              /**< Tests that must pass before this test */
    size_t num_required_tests;          /**< Number of required tests */
    char **required_features;           /**< System features required for this test */
    size_t num_required_features;       /**< Number of required features */
    
} obinexus_test_case_t;

/**
 * @brief Test execution result record
 */
typedef struct obinexus_test_execution_result {
    /* Test identification */
    char *test_id;                      /**< Test identifier */
    
    /* Execution results */
    obinexus_test_result_t result;      /**< Test execution result */
    char *result_message;               /**< Detailed result message */
    
    /* Timing information */
    uint64_t execution_time_us;         /**< Execution time in microseconds */
    uint64_t setup_time_us;             /**< Setup time in microseconds */
    uint64_t teardown_time_us;          /**< Teardown time in microseconds */
    
    /* Resource usage */
    size_t peak_memory_usage_bytes;     /**< Peak memory usage during test */
    uint32_t cpu_usage_percent;         /**< CPU usage percentage */
    
    /* Validation metrics */
    uint32_t semantic_preservation_score; /**< Semantic preservation score (0-100) */
    uint32_t performance_score;           /**< Performance score (0-100) */
    uint32_t correctness_score;           /**< Correctness score (0-100) */
    
    /* Diagnostic information */
    char *diagnostic_log;               /**< Detailed diagnostic log */
    char **diagnostic_files;            /**< Array of diagnostic file paths */
    size_t num_diagnostic_files;        /**< Number of diagnostic files */
    
} obinexus_test_execution_result_t;

/**
 * @brief Test suite definition
 */
typedef struct obinexus_test_suite {
    /* Suite identification */
    char *suite_id;                     /**< Unique suite identifier */
    char *suite_name;                   /**< Human-readable suite name */
    char *suite_description;            /**< Detailed suite description */
    
    /* Test cases */
    obinexus_test_case_t **test_cases;  /**< Array of test cases */
    size_t num_test_cases;              /**< Number of test cases */
    size_t test_case_capacity;          /**< Capacity of test case array */
    
    /* Suite configuration */
    bool parallel_execution;            /**< Enable parallel test execution */
    bool stop_on_first_failure;        /**< Stop suite execution on first failure */
    uint32_t max_concurrent_tests;      /**< Maximum concurrent test executions */
    
    /* Suite dependencies */
    char **required_suites;             /**< Suites that must complete before this suite */
    size_t num_required_suites;         /**< Number of required suites */
    
} obinexus_test_suite_t;

/**
 * @brief Test framework configuration
 */
typedef struct obinexus_test_framework_config {
    /* Execution configuration */
    bool enable_parallel_execution;     /**< Enable parallel test execution */
    uint32_t max_concurrent_tests;      /**< Maximum concurrent tests */
    uint32_t default_timeout_seconds;   /**< Default test timeout */
    
    /* Reporting configuration */
    bool generate_detailed_reports;     /**< Generate detailed test reports */
    bool preserve_diagnostic_files;     /**< Preserve diagnostic files after testing */
    char *report_output_directory;      /**< Directory for test reports */
    
    /* Validation thresholds */
    uint32_t minimum_semantic_preservation_score; /**< Minimum acceptable semantic preservation */
    uint32_t minimum_performance_score;           /**< Minimum acceptable performance */
    uint32_t minimum_correctness_score;           /**< Minimum acceptable correctness */
    
    /* Resource limits */
    size_t max_memory_usage_mb;         /**< Maximum memory usage per test */
    uint32_t max_execution_time_seconds; /**< Maximum execution time per test */
    
} obinexus_test_framework_config_t;

/**
 * @brief Complete test framework
 */
typedef struct obinexus_test_framework {
    /* Configuration */
    obinexus_test_framework_config_t *config; /**< Framework configuration */
    
    /* Test suites */
    obinexus_test_suite_t **test_suites;      /**< Array of test suites */
    size_t num_test_suites;                   /**< Number of test suites */
    size_t test_suite_capacity;               /**< Capacity of test suite array */
    
    /* Execution state */
    enum {
        OBINEXUS_FRAMEWORK_STATE_UNINITIALIZED,
        OBINEXUS_FRAMEWORK_STATE_READY,
        OBINEXUS_FRAMEWORK_STATE_RUNNING,
        OBINEXUS_FRAMEWORK_STATE_COMPLETE,
        OBINEXUS_FRAMEWORK_STATE_ERROR
    } framework_state;
    
    /* Results */
    obinexus_test_execution_result_t **results; /**< Array of test results */
    size_t num_results;                         /**< Number of test results */
    size_t result_capacity;                     /**< Capacity of results array */
    
    /* Statistics */
    struct {
        uint32_t total_tests;               /**< Total number of tests */
        uint32_t passed_tests;              /**< Number of passed tests */
        uint32_t failed_tests;              /**< Number of failed tests */
        uint32_t skipped_tests;             /**< Number of skipped tests */
        uint32_t error_tests;               /**< Number of tests with errors */
        uint64_t total_execution_time_us;   /**< Total execution time */
        double pass_rate;                   /**< Overall pass rate (0.0-1.0) */
    } statistics;
    
} obinexus_test_framework_t;

/* Core framework functions */

/**
 * @brief Create a new test framework
 * @param config Framework configuration
 * @return New test framework instance or NULL on failure
 */
obinexus_test_framework_t *
obinexus_test_framework_create(const obinexus_test_framework_config_t *config);

/**
 * @brief Add a test suite to the framework
 * @param framework The test framework
 * @param test_suite The test suite to add
 * @return true if successful, false otherwise
 */
bool
obinexus_test_framework_add_suite(
    obinexus_test_framework_t *framework,
    obinexus_test_suite_t *test_suite);

/**
 * @brief Execute all test suites in the framework
 * @param framework The test framework
 * @return true if all tests passed, false otherwise
 */
bool
obinexus_test_framework_execute_all(obinexus_test_framework_t *framework);

/**
 * @brief Execute a specific test suite
 * @param framework The test framework
 * @param suite_id The suite identifier to execute
 * @return true if suite passed, false otherwise
 */
bool
obinexus_test_framework_execute_suite(
    obinexus_test_framework_t *framework,
    const char *suite_id);

/**
 * @brief Execute a specific test case
 * @param framework The test framework
 * @param test_id The test identifier to execute
 * @return Test execution result
 */
obinexus_test_execution_result_t *
obinexus_test_framework_execute_test(
    obinexus_test_framework_t *framework,
    const char *test_id);

/**
 * @brief Generate comprehensive test report
 * @param framework The test framework
 * @param report_file_path Path for report output
 * @return true if report generated successfully, false otherwise
 */
bool
obinexus_test_framework_generate_report(
    const obinexus_test_framework_t *framework,
    const char *report_file_path);

/**
 * @brief Free test framework resources
 * @param framework The framework to free
 */
void
obinexus_test_framework_free(obinexus_test_framework_t *framework);

/* Test suite functions */

/**
 * @brief Create a new test suite
 * @param suite_id Suite identifier
 * @param suite_name Suite name
 * @param suite_description Suite description
 * @return New test suite instance or NULL on failure
 */
obinexus_test_suite_t *
obinexus_test_suite_create(
    const char *suite_id,
    const char *suite_name,
    const char *suite_description);

/**
 * @brief Add a test case to a suite
 * @param suite The test suite
 * @param test_case The test case to add
 * @return true if successful, false otherwise
 */
bool
obinexus_test_suite_add_test(
    obinexus_test_suite_t *suite,
    obinexus_test_case_t *test_case);

/**
 * @brief Free test suite resources
 * @param suite The suite to free
 */
void
obinexus_test_suite_free(obinexus_test_suite_t *suite);

/* Test case functions */

/**
 * @brief Create a new test case
 * @param test_id Test identifier
 * @param test_name Test name
 * @param test_description Test description
 * @param test_type Test type
 * @param severity Test severity
 * @return New test case instance or NULL on failure
 */
obinexus_test_case_t *
obinexus_test_case_create(
    const char *test_id,
    const char *test_name,
    const char *test_description,
    obinexus_test_type_t test_type,
    obinexus_test_severity_t severity);

/**
 * @brief Set test execution functions
 * @param test_case The test case
 * @param setup_func Setup function (can be NULL)
 * @param test_func Main test function
 * @param teardown_func Teardown function (can be NULL)
 * @return true if successful, false otherwise
 */
bool
obinexus_test_case_set_functions(
    obinexus_test_case_t *test_case,
    bool (*setup_func)(void),
    bool (*test_func)(void),
    void (*teardown_func)(void));

/**
 * @brief Free test case resources
 * @param test_case The test case to free
 */
void
obinexus_test_case_free(obinexus_test_case_t *test_case);

/* Specialized validation functions */

/**
 * @brief Validate semantic preservation across AST transformations
 * @param original_ast Original AST
 * @param transformed_bytecode Transformed bytecode
 * @param preservation_score Output for preservation score
 * @return true if validation passed, false otherwise
 */
bool
obinexus_validate_semantic_preservation(
    const void *original_ast,
    const obinexus_ast_aware_bytecode_t *transformed_bytecode,
    uint32_t *preservation_score);

/**
 * @brief Validate cross-platform compilation consistency
 * @param input_ast Input AST
 * @param platform1_bytecode Bytecode for platform 1
 * @param platform2_bytecode Bytecode for platform 2
 * @param consistency_score Output for consistency score
 * @return true if validation passed, false otherwise
 */
bool
obinexus_validate_cross_platform_consistency(
    const void *input_ast,
    const obinexus_ast_aware_bytecode_t *platform1_bytecode,
    const obinexus_ast_aware_bytecode_t *platform2_bytecode,
    uint32_t *consistency_score);

/**
 * @brief Validate performance characteristics
 * @param system The AST-Aware system
 * @param test_inputs Array of test inputs
 * @param num_inputs Number of test inputs
 * @param performance_score Output for performance score
 * @return true if validation passed, false otherwise
 */
bool
obinexus_validate_performance_characteristics(
    obinexus_ast_aware_system_t *system,
    const obinexus_compilation_input_t *test_inputs,
    size_t num_inputs,
    uint32_t *performance_score);

#endif /* OBINEXUS_VALIDATION_TESTING_FRAMEWORK_H */
