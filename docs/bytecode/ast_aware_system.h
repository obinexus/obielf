/**
 * @file ast_aware_system.h
 * @brief Complete AST-Aware Bytecode Generation System Implementation
 * 
 * This header defines the complete system integration for the AST-Aware
 * PrePost to Dual Post-Processing Bytecode Specification, providing
 * a unified interface for the entire compilation pipeline.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_AST_AWARE_SYSTEM_H
#define OBINEXUS_AST_AWARE_SYSTEM_H

#include "ast_contextualization.h"
#include "policy_attachment.h"
#include "irp_intuition_layer.h"
#include "post_processing.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief System-wide configuration parameters
 */
typedef struct obinexus_system_config {
    /* Processing mode configuration */
    bool enable_progressive_validation;     /**< Enable validation at each phase */
    bool enable_comprehensive_logging;      /**< Enable detailed operation logging */
    bool enable_performance_monitoring;     /**< Enable performance metric collection */
    
    /* Confidence and quality thresholds */
    obinexus_confidence_thresholds_t confidence_thresholds; /**< System confidence thresholds */
    uint32_t minimum_semantic_preservation_score;           /**< Minimum semantic preservation requirement */
    uint32_t minimum_traceability_score;                    /**< Minimum traceability requirement */
    
    /* Resource management */
    size_t max_memory_usage_mb;             /**< Maximum memory usage in MB */
    uint32_t max_processing_time_seconds;   /**< Maximum processing time limit */
    uint32_t max_compilation_passes;        /**< Maximum number of compilation passes */
    
    /* Debugging and diagnostics */
    bool preserve_intermediate_representations; /**< Keep intermediate forms for debugging */
    bool generate_detailed_diagnostics;         /**< Generate comprehensive diagnostic output */
    char *diagnostic_output_directory;          /**< Directory for diagnostic files */
    
} obinexus_system_config_t;

/**
 * @brief System processing statistics
 */
typedef struct obinexus_processing_statistics {
    /* Timing information */
    uint64_t contextualization_time_us;    /**< Time spent in contextualization phase */
    uint64_t policy_attachment_time_us;     /**< Time spent in policy attachment phase */
    uint64_t irp_transformation_time_us;    /**< Time spent in IRP transformation phase */
    uint64_t post_processing_time_us;       /**< Time spent in post-processing phase */
    uint64_t total_processing_time_us;      /**< Total processing time */
    
    /* Memory usage statistics */
    size_t peak_memory_usage_bytes;         /**< Peak memory usage during processing */
    size_t final_memory_usage_bytes;        /**< Final memory usage after processing */
    
    /* Processing metrics */
    uint32_t ast_nodes_processed;           /**< Number of AST nodes processed */
    uint32_t policies_applied;              /**< Number of policies applied */
    uint32_t instructions_generated;        /**< Number of bytecode instructions generated */
    uint32_t optimizations_applied;         /**< Number of optimizations applied */
    
    /* Quality metrics */
    uint32_t average_confidence_score;      /**< Average confidence across all phases */
    uint32_t semantic_preservation_score;   /**< Overall semantic preservation score */
    uint32_t traceability_completeness_score; /**< Traceability completeness score */
    
} obinexus_processing_statistics_t;

/**
 * @brief Complete AST-Aware compilation system
 */
typedef struct obinexus_ast_aware_system {
    /* System configuration */
    obinexus_system_config_t *config;       /**< System configuration */
    
    /* Processing engines */
    obinexus_context_engine_t *context_engine;         /**< AST contextualization engine */
    obinexus_policy_engine_t *policy_engine;           /**< Policy attachment engine */
    obinexus_irp_engine_t *irp_engine;                 /**< IRP transformation engine */
    obinexus_post_processing_engine_t *post_engine;    /**< Post-processing engine */
    
    /* Validation framework */
    obinexus_problem_space_validator_t *validator;     /**< Problem space validator */
    
    /* System state */
    enum {
        OBINEXUS_SYSTEM_STATE_UNINITIALIZED,
        OBINEXUS_SYSTEM_STATE_READY,
        OBINEXUS_SYSTEM_STATE_PROCESSING,
        OBINEXUS_SYSTEM_STATE_ERROR,
        OBINEXUS_SYSTEM_STATE_COMPLETE
    } system_state;
    
    /* Processing history */
    obinexus_processing_statistics_t *statistics;      /**< Processing statistics */
    char *processing_log;                              /**< Detailed processing log */
    
    /* Error handling */
    char *last_error_message;                          /**< Last error message */
    uint32_t error_code;                               /**< Last error code */
    
} obinexus_ast_aware_system_t;

/**
 * @brief Complete compilation input specification
 */
typedef struct obinexus_compilation_input {
    /* Source AST */
    void *raw_ast;                          /**< Raw AST from parser */
    char *source_file_path;                 /**< Original source file path */
    char *source_language;                  /**< Source language identifier */
    
    /* Target specification */
    obinexus_architecture_spec_t *target_architecture; /**< Target architecture */
    char *compilation_target;               /**< Compilation target identifier */
    
    /* Compilation options */
    uint32_t optimization_level;            /**< Optimization level (0-3) */
    bool enable_debug_information;          /**< Include debug information */
    bool position_independent_code;         /**< Generate position-independent code */
    
    /* Custom policies */
    obinexus_policy_attachment_t *custom_policies;     /**< Custom policy overrides */
    size_t num_custom_policies;                        /**< Number of custom policies */
    
} obinexus_compilation_input_t;

/**
 * @brief Complete compilation output specification
 */
typedef struct obinexus_compilation_output {
    /* Primary output */
    obinexus_post_processing_output_t *primary_output; /**< Primary compilation output */
    
    /* Intermediate representations (if preserved) */
    obinexus_contextualized_ast_node_t *contextualized_ast;    /**< Contextualized AST */
    obinexus_policy_bound_ast_t *policy_bound_ast;             /**< Policy-bound AST */
    obinexus_ast_aware_bytecode_t *ast_aware_bytecode;         /**< AST-Aware Bytecode */
    
    /* Compilation metadata */
    obinexus_processing_statistics_t *statistics;      /**< Processing statistics */
    char *compilation_log;                             /**< Detailed compilation log */
    
    /* Validation results */
    uint32_t overall_quality_score;         /**< Overall compilation quality score */
    char *validation_report;                /**< Comprehensive validation report */
    
    /* Diagnostic information */
    char **diagnostic_files;                /**< Array of diagnostic file paths */
    size_t num_diagnostic_files;            /**< Number of diagnostic files */
    
} obinexus_compilation_output_t;

/* Core system functions */

/**
 * @brief Create and initialize a new AST-Aware compilation system
 * @param config System configuration parameters
 * @return New system instance or NULL on failure
 */
obinexus_ast_aware_system_t *
obinexus_system_create(const obinexus_system_config_t *config);

/**
 * @brief Perform complete AST-Aware compilation
 * @param system The compilation system
 * @param input Compilation input specification
 * @param output Output container for compilation results
 * @return true if compilation successful, false otherwise
 */
bool
obinexus_system_compile(
    obinexus_ast_aware_system_t *system,
    const obinexus_compilation_input_t *input,
    obinexus_compilation_output_t **output);

/**
 * @brief Validate system configuration
 * @param config Configuration to validate
 * @param validation_report Output for validation diagnostics
 * @return true if configuration valid, false otherwise
 */
bool
obinexus_system_validate_config(
    const obinexus_system_config_t *config,
    char **validation_report);

/**
 * @brief Get current system state
 * @param system The compilation system
 * @return Current system state
 */
int
obinexus_system_get_state(const obinexus_ast_aware_system_t *system);

/**
 * @brief Get system processing statistics
 * @param system The compilation system
 * @return Processing statistics or NULL if not available
 */
const obinexus_processing_statistics_t *
obinexus_system_get_statistics(const obinexus_ast_aware_system_t *system);

/**
 * @brief Reset system state for new compilation
 * @param system The compilation system
 * @return true if reset successful, false otherwise
 */
bool
obinexus_system_reset(obinexus_ast_aware_system_t *system);

/**
 * @brief Free system resources
 * @param system The system to free
 */
void
obinexus_system_free(obinexus_ast_aware_system_t *system);

/* Utility functions */

/**
 * @brief Create default system configuration
 * @return Default configuration instance
 */
obinexus_system_config_t *
obinexus_system_config_create_default(void);

/**
 * @brief Load system configuration from file
 * @param config_file_path Path to configuration file
 * @return Loaded configuration or NULL on failure
 */
obinexus_system_config_t *
obinexus_system_config_load_from_file(const char *config_file_path);

/**
 * @brief Save system configuration to file
 * @param config Configuration to save
 * @param config_file_path Path for configuration file
 * @return true if save successful, false otherwise
 */
bool
obinexus_system_config_save_to_file(
    const obinexus_system_config_t *config,
    const char *config_file_path);

/**
 * @brief Free system configuration resources
 * @param config The configuration to free
 */
void
obinexus_system_config_free(obinexus_system_config_t *config);

/**
 * @brief Free compilation input resources
 * @param input The input to free
 */
void
obinexus_compilation_input_free(obinexus_compilation_input_t *input);

/**
 * @brief Free compilation output resources
 * @param output The output to free
 */
void
obinexus_compilation_output_free(obinexus_compilation_output_t *output);

/* System integration macros */

/**
 * @brief Macro for system-wide error checking
 */
#define OBINEXUS_SYSTEM_CHECK_ERROR(system, operation) \
    do { \
        if (!(operation)) { \
            (system)->system_state = OBINEXUS_SYSTEM_STATE_ERROR; \
            return false; \
        } \
    } while(0)

/**
 * @brief Macro for confidence threshold validation
 */
#define OBINEXUS_VALIDATE_CONFIDENCE(score, threshold, system) \
    do { \
        if ((score) < (threshold)) { \
            (system)->system_state = OBINEXUS_SYSTEM_STATE_ERROR; \
            return false; \
        } \
    } while(0)

/**
 * @brief Macro for timing measurement
 */
#define OBINEXUS_MEASURE_TIME(start_var, end_var, time_us_var) \
    do { \
        struct timespec start_time, end_time; \
        clock_gettime(CLOCK_MONOTONIC, &start_time); \
        start_var; \
        clock_gettime(CLOCK_MONOTONIC, &end_time); \
        time_us_var = (end_time.tv_sec - start_time.tv_sec) * 1000000 + \
                      (end_time.tv_nsec - start_time.tv_nsec) / 1000; \
        end_var; \
    } while(0)

#endif /* OBINEXUS_AST_AWARE_SYSTEM_H */
