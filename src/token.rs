use std::collections::BTreeMap;

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum TokenType {
    Part,
    Policy,
    Metadata,
    Bytecode,
    Custom(String),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TokenValue {
    Bytes(Vec<u8>),
    Text(String),
    Unsigned(u64),
}

#[derive(Debug, Clone, Default, PartialEq, Eq)]
pub struct TokenMemory {
    bindings: BTreeMap<String, Vec<u8>>,
}

impl TokenMemory {
    #[must_use]
    pub fn new() -> Self {
        Self::default()
    }

    pub fn bind(&mut self, key: impl Into<String>, value: impl Into<Vec<u8>>) {
        self.bindings.insert(key.into(), value.into());
    }

    #[must_use]
    pub fn get(&self, key: &str) -> Option<&[u8]> {
        self.bindings.get(key).map(Vec::as_slice)
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct UniversalToken {
    pub token_type: TokenType,
    pub value: TokenValue,
    pub memory: TokenMemory,
}

impl UniversalToken {
    #[must_use]
    pub const fn new(token_type: TokenType, value: TokenValue, memory: TokenMemory) -> Self {
        Self {
            token_type,
            value,
            memory,
        }
    }
}
