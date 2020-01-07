# wordsmith

is a forthcoming prose-checking tool written in C, which follows the following
tenets in its design:

  - Be fast. wordsmith uses the hyperscan regex library and tries to keep as
    small of a footprint as possible.

  - Be simple. No complex config files. Prose-checking rules should
    follow a relatively simple syntax:

    ```
    name;type;message;payload
    ```

    Example usage:

    ```console
    $ cat text.txt | ws -r rules.txt
    $ ws -r r1.txt -r r2.txt # two rules files, reads from stdin
    $ ws -r r1.txt -r r2.txt in1.txt in2.txt # two rules files, two input files
    ```

  - Be extensible. wordsmith will integrate libtcc to enable fast C extension
    compilation.

## Dependencies

- a c compiler
- meson
- hyperscan
- optionally, nix

## LICENSE

MIT
