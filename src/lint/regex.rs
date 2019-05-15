use crate::err::{Error::{self, *}};
use regex::{Regex, RegexSet};
use snafu::ResultExt;
use std::collections::HashMap;
use super::Linter;

struct RegexLinter {
    regex_set: RegexSet,
    memoized_regexes: HashMap<usize, Regex>,
}

impl RegexLinter {
    const name: &'static str = "regex";

    pub fn new(regexes: &[String]) -> Result<Self, Error> {
        unimplemented!()
        // let regex_set = RegexSet::new(regexes).context(LinterInit { linter: Self::name.to_string() })?;

        // Ok(RegexLinter {
        //     regex_set,
        //     memorized_regexes: HashMap::new(),
        // })
    }
}
