# wordsmith

is a prose-checking tool written in C, which follows the following
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

  - Be extensible. wordsmith integrates libtcc to allow for adding custom
    linters by specifying them in external C files. As an example:

    ```c
    // ext.c

    #include <stdlib.h>

    // We can specify custom tcc flags
    #pragma ws tcc -L/home/me/includes

    // init initializes the context of this linter
    int init(void **ctx, rules_t *rules, sink_t sink) {
        // initializing or whateva
    }

    // report goes through a piece of prose, reporting any lints
    // to the sink given previously
    int report(void *ctx, prose_t prose) {
        // reporting or whateva
    }

    // deinit frees the context initialize in init
    void deinit(void *ctx) {
        // freeing or whateva
    }
    ```

    ```console
    $ ws -e ext.c -r rules.txt prose.txt
    ```
    
For more information, wordsmith comes with a [manual](./doc/ws.1.adoc).

## Dependencies

- a c compiler
- libtcc
- meson
- hyperscan
- optionally, nix

## LICENSE

MIT
