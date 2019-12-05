# wordsmith

is a forthcoming prose-checking tool written in C, which will follow
the following design goals:

  - Be fast. wordsmith will use the hyperscan regex library

  - Be simple. No complex config files. Prose-checking rules should
    follow a relatively simple syntax:

    ```
    [name?] type [severity?] [message?] payload
    ```

    Example usage:

    ```
    $ cat text.txt | ws --rules $(cat rules.txt) --ext ext2/*.c
    ```

  - Be extensible. wordsmith is written in C to enable fast compilation
    via C. Thus, we can, at runtime, compile C extensions and include
    them.

## Dependencies

hyperscan

## LICENSE

MIT
