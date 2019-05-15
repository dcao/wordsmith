//! A Sink is an output: a place where Lints can be recorded. The most common
//! example of a Sink is just prettified command-line output. However, Sinks
//! can take multiple forms, including JSON output, binary output, etc.

pub trait Sink {
    fn recv_lint() -> Result<(), ()>;
}

pub struct CliSink {}

impl Sink for CliSink {
    fn recv_lint() -> Result<(), ()> {
        unimplemented!()
    }
}
