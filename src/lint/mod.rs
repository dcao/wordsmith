// Configuration:
// Linters and Parsers can read from a central configuration source

pub mod regex;

use crate::{err::Error, prose::{Prose, Region}, sink::Sink};

/// The severity of a lint.
#[derive(Copy, Clone)]
pub enum Severity {
    Info,
    Suggestion,
    Warning,
    Error,
}

/// A suggestion raised by a Linter.
pub struct Lint<'s> {
    linter_name: String,
    message: String,
    severity: Severity,
    region: Region<'s>,
}

impl<'s> Lint<'s> {
    pub fn linter_name(&self) -> &str {
        &self.linter_name
    }

    pub fn message(&self) -> &str {
        &self.message
    }

    pub fn severity(&self) -> Severity {
        self.severity
    }

    pub fn region(&self) -> Region<'s> {
        self.region
    }
}

/// A Linter takes a piece of Prose and lints it for errors.
pub trait Linter {
    /// The key under which this Linter should read its configuration
    /// information.
    const CONFIG_KEY: &'static str;

    fn lint<'s>(&mut self, prose: Prose<'s>, sink: impl Sink) -> Result<(), Error>;
}
