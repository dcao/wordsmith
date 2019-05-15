//! Defines the interface for Parsers, which turn text files into pieces of
//! Prose.

use crate::prose::Prose;

// TODO: Universal runtime configuration?
/// A Parser should be able to take a source file and emit a piece of Prose
/// from it.
pub trait Parser {
    /// The key under which this Parser should read its configuration
    /// information.
    const CONFIG_KEY: &'static str;

    fn can_handle(&self, filename: &str) -> bool;
    fn parse<'s>(&mut self, contents: &'s str) -> Prose<'s>;
}

/// An example parser which just returns an empty AST.
pub struct NoopParser {}

impl Parser for NoopParser {
    const CONFIG_KEY: &'static str = "noop";

    fn can_handle(&self, _filename: &str) -> bool {
        true
    }

    fn parse<'s>(&mut self, contents: &'s str) -> Prose<'s> {
        Prose::plain(contents)
    }
}
