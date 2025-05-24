/**
 * @file policy_attachment.h
 * @brief Policy Attachment Module for AST-Aware Bytecode Generation
 * 
 * This module implements the architectural decision layer that applies
 * platform-specific policies to contextualized AST nodes, ensuring
 * architecture-aware transformation with full traceability.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_POLICY_ATTACHMENT_H
#define OBINEXUS_POLICY_ATTACHMENT_H

#include "ast_contextualization.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Policy application types
 */
typedef enum {
    OBINEXUS_POLICY_ENDIANNESS,      /**< Byte order policy */
    OBINEXUS_POLICY_MEMORY_LAYOUT,   /**< Memory alignment and layout */
    OBINEXUS_POLICY_EXECUTION_MODEL, /**< Execution convention policy */
    OBINEXUS_POLICY_FEATURE_GATE,    /**< Platform feature availability */
    OBINEXUS_POLICY_ABI_CONVENTION,  /**< Application Binary Interface */
    OBINEXUS_POLICY_OPTIMIZATION,    /**< Architecture-specific optimizations */
    OBINEXUS_POLICY_SECURITY,        /**< Security model enforcement */
    OBINEXUS_POLICY_RUNTIME          /**< Runtime environment constraints */
} obinexus_policy_type_t;

/**
 * @brief Policy application priority levels
 */
typedef enum {
    OBINEXUS_POLICY_PRIORITY_CRITICAL = 100,  /**< Must be applied for correctness */
    OBINEXUS_POLICY_PRIORITY_HIGH = 80,       /**< Important for performance */
    OBINEXUS_POLICY_PRIORITY_MEDIUM = 60,     /**< Optimization opportunity */
    OBINEXUS_POLICY_PRIORITY_LOW = 40,        /**< Optional enhancement */
    OBINEXUS_POLICY_PRIORITY_INFORMATIONAL = 20 /**< Documentation only */
} obinexus_policy_priority_t;

/**
 * @brief Policy attachment record
 */
typedef struct obinexus_policy_attachment {
    obinexus_policy_type_t policy_type;      /**< Type of policy applied */
    obinexus_policy_priority_t priority;     /**< Application priority */
    uint32_t policy_id;                      /**< Unique policy identifier */
    char *policy_name;                       /**< Human-readable policy name */
    void *policy_data;                       /**< Policy-specific configuration */
    size_t policy_data_size;                 /**< Size of policy data */
    uint64_t application_timestamp;          /**< When policy was applied */
    bool mandatory;                          /**< Whether policy is mandatory */
    char *rationale;                         /**< Why this policy was applied */
    struct obinexus_policy_attachment *next; /**< Next policy in chain */
} obinexus_policy_attachment_t;

/**
 * @brief Policy-bound AST node
 */
typedef struct obinexus_policy_bound_ast {
    obinexus_contextualized_ast_node_t *contextualized_node; /**< Base contextualized node */
    obinexus_policy_attachment_t *policies;                  /**< Chain of applied policies */
    uint32_t policy_confidence;                              /**< Confidence in policy applications */
    uint64_t architectural_fingerprint;                      /**< Architecture-specific signature */
    bool policy_validation_passed;                           /**< Overall policy validation status */
    char *policy_diagnostic;                                 /**< Policy application diagnostics */
} obinexus_policy_bound_ast_t;

/**
 * @brief Architecture specification for policy application
 */
typedef struct obinexus_architecture_spec {
    char *architecture_name;              /**< Architecture identifier (e.g., "x86_64", "aarch64") */
    char *abi_name;                       /**< ABI specification (e.g., "System V", "Microsoft") */
    
    /* Endianness configuration */
    struct {
        uint8_t byte_order;               /**< 0=little, 1=big, 2=configurable */
        bool supports_mixed_endian;      /**< Whether mixed endianness is supported */
    } endianness;
    
    /* Memory layout policies */
    struct {
        uint8_t pointer_size;             /**< Pointer size in bytes */
        uint8_t natural_alignment;        /**< Natural alignment boundary */
        uint16_t max_alignment;           /**< Maximum supported alignment */
        bool requires_aligned_access;     /**< Whether unaligned access causes faults */
        size_t stack_alignment;           /**< Stack alignment requirement */
        size_t heap_alignment;            /**< Heap alignment requirement */
    } memory_layout;
    
    /* Execution model */
    struct {
        char *calling_convention;         /**< Default calling convention */
        bool supports_tail_calls;        /**< Tail call optimization support */
        uint16_t max_parameters;          /**< Maximum function parameters */
        size_t max_stack_frame;           /**< Maximum stack frame size */
        bool supports_stack_probes;       /**< Stack overflow detection support */
    } execution_model;
    
    /* Feature gates */
    struct {
        bool has_floating_point;          /**< Floating-point unit available */
        bool has_vector_instructions;     /**< SIMD/vector instructions available */
        bool has_atomic_operations;       **< Hardware atomic operations available */
        bool has_memory_barriers;         /**< Memory barrier instructions available */
        char **extension_list;            /**< List of supported ISA extensions */
        size_t num_extensions;            /**< Number of extensions in list */
    } features;
    
} obinexus_architecture_spec_t;

/**
 * @brief Policy attachment engine
 */
typedef struct obinexus_policy_engine {
    obinexus_architecture_spec_t *target_arch;  /**< Target architecture specification */
    
    /* Policy application functions */
    bool (*apply_endianness_policy)(obinexus_policy_bound_ast_t *ast_node);
    bool (*apply_memory_layout_policy)(obinexus_policy_bound_ast_t *ast_node);
    bool (*apply_execution_policy)(obinexus_policy_bound_ast_t *ast_node);
    bool (*apply_feature_gate_policy)(obinexus_policy_bound_ast_t *ast_node);
    bool (*apply_abi_policy)(obinexus_policy_bound_ast_t *ast_node);
    
    /* Policy validation functions */
    bool (*validate_policy_consistency)(const obinexus_policy_bound_ast_t *ast_node);
    uint32_t (*calculate_policy_confidence)(const obinexus_policy_bound_ast_t *ast_node);
    
    /* Configuration */
    bool enable_policy_optimization;     /**< Enable policy-based optimizations */
    bool strict_policy_enforcement;      /**< Fail on policy violations */
    uint32_t max_policies_per_node;      /**< Maximum policies per AST node */
    
} obinexus_policy_engine_t;

/**
 * @brief Create a new policy attachment engine
 * @param architecture_spec Target architecture specification
 * @return New policy engine instance or NULL on failure
 */
obinexus_policy_engine_t *
obinexus_policy_engine_create(const obinexus_architecture_spec_t *architecture_spec);

/**
 * @brief Apply policies to a contextualized AST
 * @param engine The policy engine
 * @param contextualized_ast Input contextualized AST
 * @param policy_bound_ast Output policy-bound AST
 * @return true if policy application successful, false otherwise
 */
bool
obinexus_apply_architecture_policies(
    obinexus_policy_engine_t *engine,
    const obinexus_contextualized_ast_node_t *contextualized_ast,
    obinexus_policy_bound_ast_t **policy_bound_ast);

/**
 * @brief Validate policy consistency across entire AST
 * @param policy_bound_ast The policy-bound AST to validate
 * @param validation_report Output for detailed validation report
 * @return Overall policy validation confidence (0-100)
 */
uint32_t
obinexus_validate_policy_consistency(
    const obinexus_policy_bound_ast_t *policy_bound_ast,
    char **validation_report);

/**
 * @brief Generate architectural fingerprint for policy-bound AST
 * @param policy_bound_ast The AST to fingerprint
 * @param fingerprint Output buffer for architectural signature
 * @param fingerprint_size Size of output buffer
 * @return Number of bytes written to fingerprint buffer
 */
size_t
obinexus_generate_architectural_fingerprint(
    const obinexus_policy_bound_ast_t *policy_bound_ast,
    uint8_t *fingerprint,
    size_t fingerprint_size);

/**
 * @brief Free policy-bound AST resources
 * @param policy_bound_ast The AST to free
 */
void
obinexus_policy_bound_ast_free(obinexus_policy_bound_ast_t *policy_bound_ast);

/**
 * @brief Free policy engine resources
 * @param engine The engine to free
 */
void
obinexus_policy_engine_free(obinexus_policy_engine_t *engine);

/* Policy-specific data structures */

/**
 * @brief Endianness policy data
 */
typedef struct obinexus_endianness_policy {
    uint8_t target_byte_order;           /**< Target endianness (0=little, 1=big) */
    bool conversion_required;            /**< Whether byte order conversion needed */
    uint32_t affected_data_types;        /**< Bitmask of affected data types */
    bool preserve_source_order;          /**< Whether to preserve source byte order */
} obinexus_endianness_policy_t;

/**
 * @brief Memory layout policy data
 */
typedef struct obinexus_memory_layout_policy {
    uint8_t struct_alignment;            /**< Structure alignment requirement */
    uint8_t array_alignment;             /**< Array alignment requirement */
    bool pack_structs;                   /**< Whether to pack structures */
    uint16_t padding_byte_value;         /**< Value to use for padding bytes */
    bool optimize_layout;                /**< Whether to optimize member ordering */
} obinexus_memory_layout_policy_t;

/**
 * @brief Execution convention policy data
 */
typedef struct obinexus_execution_policy {
    char *calling_convention;            /**< Calling convention to use */
    bool enable_tail_call_optimization;  /**< Enable tail call optimization */
    uint16_t parameter_passing_limit;    /**< Maximum parameters in registers */
    bool use_frame_pointer;              /**< Whether to maintain frame pointer */
    char *exception_model;               /**< Exception handling model */
} obinexus_execution_policy_t;

/**
 * @brief Feature gate policy data
 */
typedef struct obinexus_feature_gate_policy {
    char **required_features;            /**< List of required CPU features */
    size_t num_required_features;        /**< Number of required features */
    char **optional_features;            /**< List of optional CPU features */
    size_t num_optional_features;        /**< Number of optional features */
    bool fallback_implementation;        /**< Whether fallback exists for missing features */
    uint32_t minimum_cpu_level;          /**< Minimum CPU capability level required */
} obinexus_feature_gate_policy_t;

#endif /* OBINEXUS_POLICY_ATTACHMENT_H */