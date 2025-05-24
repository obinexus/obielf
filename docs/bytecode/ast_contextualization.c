/**
 * @file ast_contextualization.h
 * @brief AST Contextualization Layer for OBINexus AST-Aware Bytecode System
 * 
 * This module implements the semantic enrichment phase that transforms raw
 * parser AST output into policy-augmented AST with contextual encoding.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_AST_CONTEXTUALIZATION_H
#define OBINEXUS_AST_CONTEXTUALIZATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Context enrichment types for AST nodes
 */
typedef enum {
    OBINEXUS_CONTEXT_TYPE_SYSTEM,     /**< Type system enforcement */
    OBINEXUS_CONTEXT_SCOPE,           /**< Scope boundary definition */
    OBINEXUS_CONTEXT_POLICY,          /**< Language policy application */
    OBINEXUS_CONTEXT_PLATFORM,       /**< Platform-specific constraints */
    OBINEXUS_CONTEXT_SEMANTIC,       /**< Semantic validation rules */
    OBINEXUS_CONTEXT_EXECUTION        /**< Execution environment context */
} obinexus_context_type_t;

/**
 * @brief Context annotation structure for AST nodes
 */
typedef struct obinexus_ast_context {
    obinexus_context_type_t type;     /**< Context type identifier */
    uint32_t confidence_level;        /**< Confidence in context accuracy (0-100) */
    uint64_t semantic_hash;           /**< Semantic fingerprint for validation */
    void *context_data;               /**< Type-specific context information */
    size_t context_data_size;         /**< Size of context data */
    struct obinexus_ast_context *next;/**< Linked list for multiple contexts */
} obinexus_ast_context_t;

/**
 * @brief Enhanced AST node with contextualization
 */
typedef struct obinexus_contextualized_ast_node {
    void *original_ast_node;          /**< Reference to original parser AST node */
    obinexus_ast_context_t *contexts; /**< Chain of applied contexts */
    uint32_t total_confidence;        /**< Aggregate confidence score */
    uint64_t lineage_id;              /**< Unique identifier for traceability */
    bool validation_passed;           /**< Context validation status */
    char *diagnostic_info;            /**< Human-readable diagnostic information */
} obinexus_contextualized_ast_node_t;

/**
 * @brief Context enrichment engine
 */
typedef struct obinexus_context_engine {
    /* Type system integration */
    struct {
        bool (*validate_type_constraints)(const void *ast_node, void *type_context);
        void *(*infer_type_information)(const void *ast_node);
        bool (*check_type_compatibility)(const void *source, const void *target);
    } type_system;
    
    /* Scope analysis */
    struct {
        void *(*create_scope_context)(const void *ast_node);
        bool (*validate_scope_access)(const void *ast_node, void *scope_context);
        void (*update_scope_boundaries)(void *scope_context, const void *ast_node);
    } scope_analyzer;
    
    /* Policy enforcement */
    struct {
        bool (*apply_language_policy)(void *ast_node, const char *policy_name);
        void *(*get_applicable_policies)(const void *ast_node);
        bool (*validate_policy_compliance)(const void *ast_node, void *policy_set);
    } policy_engine;
    
    /* Platform constraints */
    struct {
        void *(*get_platform_constraints)(const char *target_platform);
        bool (*validate_platform_compatibility)(const void *ast_node, void *constraints);
        void (*apply_platform_adaptations)(void *ast_node, void *constraints);
    } platform_adapter;
    
    /* Configuration */
    uint32_t max_context_depth;      /**< Maximum context nesting level */
    double min_confidence_threshold; /**< Minimum confidence for progression */
    bool enable_diagnostic_output;   /**< Enable detailed diagnostic generation */
    
} obinexus_context_engine_t;

/**
 * @brief Create a new AST contextualization engine
 * @param config Engine configuration parameters
 * @return New context engine instance or NULL on failure
 */
obinexus_context_engine_t *
obinexus_context_engine_create(const void *config);

/**
 * @brief Apply contextualization to a raw AST
 * @param engine The contextualization engine
 * @param raw_ast The raw AST from parser
 * @param target_platform Target platform identifier
 * @param contextualized_ast Output for contextualized AST
 * @return true if contextualization successful, false otherwise
 */
bool
obinexus_ast_apply_contextualization(
    obinexus_context_engine_t *engine,
    const void *raw_ast,
    const char *target_platform,
    obinexus_contextualized_ast_node_t **contextualized_ast);

/**
 * @brief Validate context consistency across AST
 * @param contextualized_ast The contextualized AST to validate
 * @param validation_report Output for validation diagnostics
 * @return Overall confidence score (0-100)
 */
uint32_t
obinexus_ast_validate_context_consistency(
    const obinexus_contextualized_ast_node_t *contextualized_ast,
    char **validation_report);

/**
 * @brief Extract semantic fingerprint from contextualized AST
 * @param contextualized_ast The AST to analyze
 * @param fingerprint Output buffer for semantic hash
 * @param fingerprint_size Size of output buffer
 * @return Number of bytes written to fingerprint buffer
 */
size_t
obinexus_ast_extract_semantic_fingerprint(
    const obinexus_contextualized_ast_node_t *contextualized_ast,
    uint8_t *fingerprint,
    size_t fingerprint_size);

/**
 * @brief Free contextualized AST resources
 * @param contextualized_ast The AST to free
 */
void
obinexus_contextualized_ast_free(obinexus_contextualized_ast_node_t *contextualized_ast);

/**
 * @brief Free context engine resources
 * @param engine The engine to free
 */
void
obinexus_context_engine_free(obinexus_context_engine_t *engine);

/* Context-specific data structures */

/**
 * @brief Type system context data
 */
typedef struct obinexus_type_context {
    char *type_name;                  /**< Resolved type name */
    uint32_t type_id;                 /**< Unique type identifier */
    size_t type_size;                 /**< Size in bytes on target platform */
    uint16_t alignment_requirement;   /**< Memory alignment requirement */
    bool is_pointer_type;             /**< Whether this is a pointer type */
    bool is_aggregate_type;           /**< Whether this is struct/array/union */
    void *type_metadata;              /**< Additional type-specific metadata */
} obinexus_type_context_t;

/**
 * @brief Scope context data
 */
typedef struct obinexus_scope_context {
    uint32_t scope_id;                /**< Unique scope identifier */
    uint32_t parent_scope_id;         /**< Parent scope identifier */
    uint16_t nesting_level;           /**< Scope nesting depth */
    void *symbol_table;               /**< Symbols visible in this scope */
    uint64_t scope_flags;             /**< Scope property flags */
} obinexus_scope_context_t;

/**
 * @brief Platform constraint data
 */
typedef struct obinexus_platform_constraint {
    char *platform_name;              /**< Target platform identifier */
    uint8_t endianness;               /**< 0=little, 1=big, 2=configurable */
    uint8_t pointer_size;             /**< Pointer size in bytes */
    uint16_t natural_alignment;       /**< Natural alignment boundary */
    uint32_t max_stack_frame;         /**< Maximum stack frame size */
    bool supports_unaligned_access;   /**< Unaligned memory access support */
    void *abi_specifics;              /**< ABI-specific constraint data */
} obinexus_platform_constraint_t;

#endif /* OBINEXUS_AST_CONTEXTUALIZATION_H */