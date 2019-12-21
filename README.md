# wordsmith

is a prose-checking tool written in Zig, which adheres to the following
design goals:

  - Be fast. wordsmith uses the hyperscan regex library to this end.
    (A future release might make this optional, falling back to a slower
    regex engine when needed)

  - Be simple. No complex config files. Prose-checking rules are as
    simple as possible:

    ```
    name;type;message;payload
    ```

    Example usage:

    ```
    $ cat text.txt | ws -i --rules rules.txt --ext ext2/*.c
    ```

  - Be extensible. wordsmith is structured to make adding custom lints
    relatively straightforward. In the future, wordsmith will also
    be able to dynamically load object files with extra lints defined.

## Dependencies

  - hyperscan
  - zig compiler
  - Nix (if used for building/developing)

## Prior art

  - [vale](https://github.com/errata-ai/vale)
  - [proselint](https://github.com/amperser/proselint)

## LICENSE

MIT
