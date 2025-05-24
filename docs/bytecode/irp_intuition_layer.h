/**
 * @file irp_intuition_layer.h
 * @brief IRP Intuition Layer - AST-Aware Bytecode Interpolation Engine
 * 
 * This module implements the core transformation engine that converts
 * policy-bound AST into AST-Aware Bytecode while preserving semantic
 * lineage and enabling architecture-specific optimizations.
 * 
 * @copyright Copyright (c) 2025 OBINexus Computing
 * @license Proprietary - All Rights Reserved
 */

#ifndef OBINEXUS_IRP_INTUITION_LAYER_H
#define OBINEXUS_IRP_INTUITION_LAYER_H

#include "policy_attachment.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief IRP bytecode instruction types
 */
typedef enum {
    /* Data movement instructions */
    OBINEXUS_IRP_LOAD,          /**< Load from memory/register */
    OBINEXUS_IRP_STORE,         /**< Store to memory/register */
    OBINEXUS_IRP_MOVE,          /**< Move between locations */
    OBINEXUS_IRP_COPY,          /**< Copy data with size specification */
    
    /* Arithmetic instructions */
    OBINEXUS_IRP_ADD,           /**< Addition operation */
    OBINEXUS_IRP_SUB,           /**< Subtraction operation */
    OBINEXUS_IRP_MUL,           /**< Multiplication operation */
    OBINEXUS_IRP_DIV,           /**< Division operation */
    OBINEXUS_IRP_MOD,           /**< Modulo operation */
    
    /* Logical instructions */
    OBINEXUS_IRP_AND,           /**< Bitwise AND */
    OBINEXUS_IRP_OR,            /**< Bitwise OR */
    OBINEXUS_IRP_XOR,           /**< Bitwise XOR */
    OBINEXUS_IRP_NOT,           /**< Bitwise NOT */
    OBINEXUS_IRP_SHIFT_LEFT,    /**< Left shift */
    OBINEXUS_IRP_SHIFT_RIGHT,   /**< Right shift */
    
    /* Control flow instructions */
    OBINEXUS_IRP_JUMP,          /**< Unconditional jump */
    OBINEXUS_IRP_JUMP_IF,       /**< Conditional jump */
    OBINEXUS_IRP_CALL,          /**< Function call */
    OBINEXUS_IRP_RETURN,        /**< Function return */
    OBINEXUS_IRP_LOOP,          /**< Loop construct */
    
    /* Memory management */
    OBINEXUS_IRP_ALLOC,         /**< Memory allocation */
    OBINEXUS_IRP_FREE,          /**< Memory deallocation */
    OBINEXUS_IRP_STACK_ALLOC,   /**< Stack allocation */
    OBINEXUS_IRP_STACK_FREE,    /**< Stack deallocation */
    
    /* Type operations */
    OBINEXUS_IRP_CAST,          /**< Type casting */
    OBINEXUS_IRP_SIZEOF,        /**< Size calculation */
    OBINEXUS_IRP_TYPEOF,        /**< Type information */
    
    /* AST-specific instructions */
    OBINEXUS_IRP_AST_ANCHOR,    /**< AST node reference point */
    OBINEXUS_IRP_SEMANTIC_MARK, /**< Semantic boundary marker */
    OBINEXUS_IRP_POLICY_MARK,   /**< Policy application marker */
    OBINEXUS_IRP_DEBUG_INFO,    /**< Debug information embedding */
    
    /* Architecture-specific */
    OBINEXUS_IRP_ARCH_SPECIFIC, /**< Architecture-specific instruction */
    OBINEXUS_IRP_OPTIMIZATION   /**< Optimization hint */
    
} obinexus_irp_instruction_type_t;

/**
 * @brief IRP operand types
 */
typedef enum {
    OBINEXUS_IRP_OPERAND_IMMEDIATE,     /**< Immediate value */
    OBINEXUS_IRP_OPERAND_REGISTER,      /**< Virtual register */
    OBINEXUS_IRP_OPERAND_MEMORY,        /**< Memory location */
    OBINEXUS_IRP_OPERAND_LABEL,         /**< Code label */
    OBINEXUS_IRP_OPERAND_AST_REF,       /**< AST node reference */
    OBINEXUS_IRP_OPERAND_TYPE_REF,      /**< Type information reference */
    OBINEXUS_IRP_OPERAND_POLICY_REF     /**< Policy reference */
} obinexus_irp_operand_type_t;

/**
 * @brief IRP instruction operand
 */
typedef struct obinexus_irp_operand {
    obinexus_irp_operand_type_t type;   /**< Operand type */
    union {
        struct {
            uint64_t value;             /**< Immediate value */
            uint8_t size;               /**< Value size in bytes */
        } immediate;
        
        struct {
            uint32_t reg_id;            /**< Virtual register ID */
            uint8_t reg_class;          /**< Register class (general, float, etc.) */
        } reg;
        
        struct {
            uint64_t address;           /**< Memory address or offset */
            uint32_t base_reg;          /**< Base register for relative addressing */
            uint8_t size;               /**< Access size in bytes */
        } memory;
        
        struct {
            uint32_t label_id;          /**< Label identifier */
            char *label_name;           /**< Human-readable label name */
        } label;
        
        struct {
            uint64_t ast_node_id;       /**< AST node identifier */
            uint32_t node_type;         /**< AST node type */
        } ast_ref;
        
        struct {
            uint32_t type_id;           /**< Type identifier */
            char *type_name;            /**< Type name */
        } type_ref;
        
        struct {
            uint32_t policy_id;         /**< Policy identifier */
            char *policy_name;          /**< Policy name */
        } policy_ref;
    } data;
} obinexus_irp_operand_t;

/**
 * @brief IRP bytecode instruction
 */
typedef struct obinexus_irp_instruction {
    obinexus_irp_instruction_type_t opcode;     /**< Instruction opcode */
    uint32_t instruction_id;                    /**< Unique instruction identifier */
    
    /* Operands */
    obinexus_irp_operand_t *operands;           /**< Array of operands */
    uint8_t num_operands;                       /**< Number of operands */
    
    /* AST lineage information */
    uint64_t source_ast_node_id;                /**< Originating AST node ID */
    uint32_t ast_node_type;                     /**< AST node type */
    uint64_t semantic_context_id;               /**< Semantic context identifier */
    
    /* Policy information */
    uint32_t *applied_policies;                 /**< Array of applied policy IDs */
    uint8_t num_applied_policies;               /**< Number of applied policies */
    
    /* Architecture information */
    char *target_architecture;                  /**< Target architecture name */
    uint32_t architectural_flags;               /**< Architecture-specific flags */
    
    /* Debug and traceability */
    uint32_t source_line;                       /**< Source code line number */
    uint32_t source_column;                     /**< Source code column number */
    char *debug_comment;                        /**< Human-readable debug comment */
    
    /* Instruction metadata */
    uint64_t generation_timestamp;              /**< When instruction was generated */
    uint32_t confidence_score;                  /**< Confidence in instruction correctness */
    bool optimization_barrier;                  /**< Whether this instruction blocks optimizations */
    
} obinexus_irp_instruction_t;

/**
 * @brief AST-Aware Bytecode container
 */
typedef struct obinexus_ast_aware_bytecode {
    /* Instruction sequence */
    obinexus_irp_instruction_t *instructions;   /**< Array of IRP instructions */
    size_t num_instructions;                    /**< Number of instructions */
    size_t instruction_capacity;                /**< Capacity of instruction array */
    
    /* AST lineage mapping */
    struct {
        uint64_t *ast_node_ids;                 /**< Array of AST node IDs */
        uint32_t *instruction_ranges;           /**< Instruction ranges for each AST node */
        size_t num_ast_nodes;                   /**< Number of mapped AST nodes */
    } lineage_map;
    
    /* Policy application record */
    struct {
        uint32_t *policy_ids;                   /**< Array of applied policy IDs */
        char **policy_names;                    /**< Array of policy names */
        uint32_t *instruction_indices;          /**< Instructions affected by each policy */
        size_t num_policies;                    /**< Number of applied policies */
    } policy_record;
    
    /* Architecture information */
    obinexus_architecture_spec_t *target_arch;  /**< Target architecture specification */
    uint64_t architectural_fingerprint;         /**< Architecture-specific signature */
    
    /* Semantic preservation */
    uint64_t semantic_hash;                     /**< Semantic content hash */
    uint32_t semantic_version;                  /**< Semantic encoding version */
    bool semantic_integrity_verified;          /**< Semantic integrity status */
    
    /* Metadata */
    char *source_file;                          /**< Original source file name */
    uint64_t generation_timestamp;              /**< Bytecode generation timestamp */
    char *compiler_version;                     /**< Compiler version identifier */
    uint32_t overall_confidence;                /**< Overall bytecode confidence */
    
} obinexus_ast_aware_bytecode_t;

/**
 * @brief IRP transformation engine
 */
typedef struct obinexus_irp_engine {
    /* Transformation functions */
    bool (*transform_ast_node)(const obinexus_policy_bound_ast_t *ast_node,
                              obinexus_irp_instruction_t **instructions,
                              size_t *num_instructions);
    
    bool (*apply_architecture_optimizations)(obinexus_ast_aware_bytecode_t *bytecode);
    
    bool (*validate_semantic_preservation)(const obinexus_policy_bound_ast_t *original_ast,
                                          const obinexus_ast_aware_bytecode_t *bytecode);
    
    /* Lineage tracking */
    bool (*create_lineage_mapping)(const obinexus_policy_bound_ast_t *ast,
                                  obinexus_ast_aware_bytecode_t *bytecode);
    
    uint64_t (*generate_semantic_hash)(const obinexus_ast_aware_bytecode_t *bytecode);
    
    /* Configuration */
    bool enable_optimization;                   /**< Enable bytecode optimizations */
    bool preserve_debug_info;                   /**< Preserve debug information */
    bool validate_lineage;                      /**< Validate AST lineage mapping */
    uint32_t max_optimization_passes;          /**< Maximum optimization iterations */
    
} obinexus_irp_engine_t;

/**
 * @brief Create a new IRP transformation engine
 * @param target_arch Target architecture specification
 * @return New IRP engine instance or NULL on failure
 */
obinexus_irp_engine_t *
obinexus_irp_engine_create(const obinexus_architecture_spec_t *target_arch);

/**
 * @brief Transform policy-bound AST into AST-Aware Bytecode
 * @param engine The IRP transformation engine
 * @param policy_bound_ast Input policy-bound AST
 * @param ast_aware_bytecode Output AST-Aware Bytecode
 * @return true if transformation successful, false otherwise
 */
bool
obinexus_irp_transform_to_bytecode(
    obinexus_irp_engine_t *engine,
    const obinexus_policy_bound_ast_t *policy_bound_ast,
    obinexus_ast_aware_bytecode_t **ast_aware_bytecode);

/**
 * @brief Validate bytecode semantic integrity
 * @param original_ast Original policy-bound AST
 * @param bytecode Generated AST-Aware Bytecode
 * @param validation_report Output for detailed validation report
 * @return Semantic integrity confidence score (0-100)
 */
uint32_t
obinexus_validate_bytecode_integrity(
    const obinexus_policy_bound_ast_t *original_ast,
    const obinexus_ast_aware_bytecode_t *bytecode,
    char **validation_report);

/**
 * @brief Generate portable bytecode representation
 * @param bytecode AST-Aware Bytecode to serialize
 * @param portable_format Output buffer for portable format
 * @param format_size Size of output buffer
 * @return Number of bytes written to portable format
 */
size_t
obinexus_generate_portable_bytecode(
    const obinexus_ast_aware_bytecode_t *bytecode,
    uint8_t *portable_format,
    size_t format_size);

/**
 * @brief Free AST-Aware Bytecode resources
 * @param bytecode The bytecode to free
 */
void
obinexus_ast_aware_bytecode_free(obinexus_ast_aware_bytecode_t *bytecode);

/**
 * @brief Free IRP engine resources
 * @param engine The engine to free
 */
void
obinexus_irp_engine_free(obinexus_irp_engine_t *engine);

#endif /* OBINEXUS_IRP_INTUITION_LAYER_H */
