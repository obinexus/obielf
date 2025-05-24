/**
 * @file post_processing.h
 * @brief Post-Processing Dual-Path Architecture for AST-Aware Bytecode
 * 
 * This module implements the dual-path post-processing system that provides
 * both AST Assembly Validation and AXC Mode executable generation based on
 * confidence levels and validation requirements.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_POST_PROCESSING_H
#define OBINEXUS_POST_PROCESSING_H

#include "irp_intuition_layer.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Post-processing output modes
 */
typedef enum {
    OBINEXUS_OUTPUT_AST_ASSEMBLY,       /**< AST Assembly validation model */
    OBINEXUS_OUTPUT_AXC_MODE,           /**< Architecture-Confident executable */
    OBINEXUS_OUTPUT_HYBRID,             /**< Both AST Assembly and AXC Mode */
    OBINEXUS_OUTPUT_INTROSPECTIVE_IR    /**< Full introspectable IR with AST links */
} obinexus_output_mode_t;

/**
 * @brief Confidence thresholds for output mode selection
 */
typedef struct obinexus_confidence_thresholds {
    uint32_t axc_mode_threshold;        /**< Minimum confidence for AXC Mode (90-100) */
    uint32_t hybrid_mode_threshold;     /**< Minimum confidence for Hybrid mode (70-89) */
    uint32_t assembly_mode_threshold;   /**< Minimum confidence for Assembly mode (50-69) */
    uint32_t failure_threshold;         /**< Below this, compilation fails (<50) */
} obinexus_confidence_thresholds_t;

/**
 * @brief AST Assembly validation record
 */
typedef struct obinexus_ast_assembly_record {
    /* Assembly generation information */
    char *assembly_code;                /**< Generated assembly code */
    size_t assembly_size;               /**< Size of assembly code */
    char *target_assembly_syntax;       /**< Assembly syntax (AT&T, Intel, etc.) */
    
    /* AST lineage preservation */
    struct {
        uint64_t *ast_node_ids;         /**< AST node identifiers */
        uint32_t *assembly_line_ranges; /**< Assembly line ranges for each AST node */
        char **semantic_annotations;    /**< Semantic annotations for assembly lines */
        size_t num_mappings;            /**< Number of AST-to-assembly mappings */
    } lineage_mapping;
    
    /* Validation information */
    struct {
        bool semantic_equivalence_verified; /**< Semantic equivalence validation status */
        bool control_flow_verified;         /**< Control flow validation status */
        bool data_flow_verified;            /**< Data flow validation status */
        uint32_t validation_confidence;     /**< Overall validation confidence */
        char *validation_report;            /**< Detailed validation report */
    } validation_status;
    
    /* Debugging support */
    struct {
        bool debug_symbols_included;    /**< Debug symbol inclusion status */
        bool line_number_mapping;       /**< Source line number mapping */
        bool variable_name_preservation;/**< Variable name preservation */
        char *debug_format;             /**< Debug information format */
    } debug_info;
    
    /* Traceability information */
    uint64_t generation_timestamp;      /**< Assembly generation timestamp */
    char *compiler_fingerprint;        /**< Compiler version and configuration */
    uint64_t semantic_hash;             /**< Semantic content verification hash */
    
} obinexus_ast_assembly_record_t;

/**
 * @brief AXC Mode executable record
 */
typedef struct obinexus_axc_executable_record {
    /* Executable information */
    uint8_t *executable_data;           /**< Binary executable data */
    size_t executable_size;             /**< Size of executable data */
    char *executable_format;            /**< Executable format (ELF, PE, Mach-O, etc.) */
    char *target_platform;              /**< Target platform specification */
    
    /* Architecture-specific information */
    struct {
        char *instruction_set;          /**< Target instruction set */
        char *abi_specification;        /**< ABI specification used */
        uint32_t optimization_level;    /**< Applied optimization level */
        bool position_independent;      /**< Position-independent code status */
        char **used_features;           /**< CPU features utilized */
        size_t num_features;            /**< Number of CPU features */
    } architecture_info;
    
    /* Trust and verification */
    struct {
        uint32_t trust_level;           /**< Trust level (90-100 for AXC Mode) */
        bool integrity_verified;        /**< Executable integrity verification */
        bool performance_validated;     /**< Performance characteristics validated */
        char *verification_certificate; /**< Digital verification certificate */
        uint64_t trust_fingerprint;     /**< Trust verification fingerprint */
    } trust_info;
    
    /* Deployment information */
    struct {
        char **dependencies;            /**< Runtime dependencies */
        size_t num_dependencies;        /**< Number of dependencies */
        char *deployment_target;        /**< Intended deployment environment */
        bool self_contained;            /**< Self-contained executable status */
    } deployment_info;
    
    /* Metadata */
    uint64_t generation_timestamp;      /**< Executable generation timestamp */
    char *build_configuration;         /**< Build configuration used */
    uint64_t build_hash;               /**< Build reproducibility hash */
    
} obinexus_axc_executable_record_t;

/**
 * @brief Introspectable IR record
 */
typedef struct obinexus_introspectable_ir_record {
    /* IR representation */
    char *ir_code;                      /**< Intermediate representation code */
    size_t ir_size;                     /**< Size of IR code */
    char *ir_format;                    /**< IR format specification */
    
    /* Complete AST linkage */
    struct {
        obinexus_ast_aware_bytecode_t *original_bytecode; /**< Original AST-Aware Bytecode */
        uint64_t *complete_ast_mapping;                    /**< Complete AST node mapping */
        char **semantic_context_info;                     /**< Semantic context for each IR element */
        size_t num_ir_elements;                           /**< Number of IR elements */
    } ast_linkage;
    
    /* Policy trace */
    struct {
        uint32_t *policy_application_trace;    /**< Policy application trace */
        char **policy_decision_rationale;      /**< Rationale for each policy decision */
        size_t num_policy_decisions;           /**< Number of policy decisions */
    } policy_trace;
    
    /* Debugging and analysis support */
    struct {
        bool supports_step_debugging;     /**< Step-by-step debugging support */
        bool supports_state_inspection;   /**< Runtime state inspection support */
        bool supports_semantic_queries;   /**< Query support for semantic information */
        char *analysis_tools_format;      /**< Format for analysis tools integration */
    } analysis_support;
    
} obinexus_introspectable_ir_record_t;

/**
 * @brief Post-processing output container
 */
typedef struct obinexus_post_processing_output {
    obinexus_output_mode_t output_mode;          /**< Selected output mode */
    uint32_t overall_confidence;                 /**< Overall processing confidence */
    
    /* Output records (based on mode) */
    obinexus_ast_assembly_record_t *assembly_record;     /**< AST Assembly record (if applicable) */
    obinexus_axc_executable_record_t *executable_record; /**< AXC executable record (if applicable) */
    obinexus_introspectable_ir_record_t *ir_record;      /**< Introspectable IR record (if applicable) */
    
    /* Processing metadata */
    struct {
        uint64_t processing_start_time;     /**< Processing start timestamp */
        uint64_t processing_end_time;       /**< Processing completion timestamp */
        uint32_t processing_passes;         /**< Number of processing passes performed */
        char *processing_log;               /**< Detailed processing log */
    } processing_metadata;
    
    /* Quality metrics */
    struct {
        uint32_t semantic_preservation_score;  /**< Semantic preservation quality (0-100) */
        uint32_t traceability_score;           /**< Traceability quality (0-100) */
        uint32_t performance_score;            /**< Performance quality (0-100) */
        uint32_t portability_score;            /**< Portability quality (0-100) */
    } quality_metrics;
    
} obinexus_post_processing_output_t;

/**
 * @brief Post-processing engine
 */
typedef struct obinexus_post_processing_engine {
    /* Configuration */
    obinexus_confidence_thresholds_t confidence_thresholds; /**< Confidence thresholds */
    obinexus_architecture_spec_t *target_architecture;      /**< Target architecture */
    
    /* Processing functions */
    bool (*generate_ast_assembly)(const obinexus_ast_aware_bytecode_t *bytecode,
                                 obinexus_ast_assembly_record_t **assembly_record);
    
    bool (*generate_axc_executable)(const obinexus_ast_aware_bytecode_t *bytecode,
                                   obinexus_axc_executable_record_t **executable_record);
    
    bool (*generate_introspectable_ir)(const obinexus_ast_aware_bytecode_t *bytecode,
                                      obinexus_introspectable_ir_record_t **ir_record);
    
    /* Validation functions */
    bool (*validate_semantic_equivalence)(const obinexus_ast_aware_bytecode_t *original,
                                         const obinexus_ast_assembly_record_t *assembly);
    
    bool (*validate_executable_integrity)(const obinexus_axc_executable_record_t *executable);
    
    uint32_t (*calculate_trust_level)(const obinexus_ast_aware_bytecode_t *bytecode);
    
    /* Configuration options */
    bool enable_optimization;               /**< Enable post-processing optimizations */
    bool require_validation;                /**< Require validation before output */
    bool preserve_all_debug_info;          /**< Preserve complete debug information */
    uint32_t max_processing_passes;        /**< Maximum processing iterations */
    
} obinexus_post_processing_engine_t;

/**
 * @brief Create a new post-processing engine
 * @param target_arch Target architecture specification
 * @param confidence_thresholds Confidence threshold configuration
 * @return New post-processing engine instance or NULL on failure
 */
obinexus_post_processing_engine_t *
obinexus_post_processing_engine_create(
    const obinexus_architecture_spec_t *target_arch,
    const obinexus_confidence_thresholds_t *confidence_thresholds);

/**
 * @brief Process AST-Aware Bytecode through dual-path post-processing
 * @param engine The post-processing engine
 * @param ast_aware_bytecode Input AST-Aware Bytecode
 * @param output Output container for processing results
 * @return true if processing successful, false otherwise
 */
bool
obinexus_post_process_bytecode(
    obinexus_post_processing_engine_t *engine,
    const obinexus_ast_aware_bytecode_t *ast_aware_bytecode,
    obinexus_post_processing_output_t **output);

/**
 * @brief Validate post-processing output quality
 * @param output Post-processing output to validate
 * @param validation_report Output for detailed validation report
 * @return Overall output quality score (0-100)
 */
uint32_t
obinexus_validate_post_processing_quality(
    const obinexus_post_processing_output_t *output,
    char **validation_report);

/**
 * @brief Free post-processing output resources
 * @param output The output to free
 */
void
obinexus_post_processing_output_free(obinexus_post_processing_output_t *output);

/**
 * @brief Free post-processing engine resources
 * @param engine The engine to free
 */
void
obinexus_post_processing_engine_free(obinexus_post_processing_engine_t *engine);

#endif /* OBINEXUS_POST_PROCESSING_H */