//! Linters and Parsers can choose to read from runtime configuration files in
//! order to avoid having to mess about with configuring Linters or Parsers via
//! source code.
//!
//! The configuration should be written in the YAML format, and at the top level
//! there can only be any of two keys: "parser" and "linter," corresponding to
//! parser and linter settings, respectively.

use serde::{Deserialize, Serialize};
use serde_yaml::Mapping;
use std::collections::HashMap;

#[derive(Debug, Deserialize, Serialize)]
struct Config {
    parser: HashMap<String, Mapping>,
    linter: HashMap<String, Mapping>,
}
