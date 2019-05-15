use wordsmith::cli::{cli, WordsmithConfig};

fn main() {
    let cfg = WordsmithConfig {};
    cli(&cfg).get_matches();
}
