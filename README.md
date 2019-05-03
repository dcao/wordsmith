# wordsmith

is a forthcoming prose-checking tool, with the following goals:

  - Be fast. SAX-style parsing/error-reporting/etc. Avoid lots of
    copying.
  
  - Allow for user customization by providing a kit to "build-your-
    own-prose-checker," ideally like so:
    
    ```rust
    fn main() {
        wordsmith! {
            parsers: [txt, md, org];
            linters: [proselint, like-check, sentence-complexity];
        }
    }
    ```
    
    (impl note: parsers/ mod, linters/ mod)
