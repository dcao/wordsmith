/// Defines the command-line interface for a wordsmith application.
use clap::{crate_authors, crate_description, crate_name, crate_version, App};

pub struct WordsmithConfig {}

pub fn cli(ws: &WordsmithConfig) -> App {
    let app = App::new(crate_name!())
        .version(crate_version!())
        .author(crate_authors!("\n"))
        .about(crate_description!());

    app
}
