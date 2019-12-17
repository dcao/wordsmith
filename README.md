# wordsmith

is a forthcoming prose-checking tool written in Zig, which will follow
the following design goals:

  - Be fast. wordsmith will use the hyperscan regex library

  - Be simple. No complex config files. Prose-checking rules should
    follow a relatively simple syntax:

    ```
    [name?] type [severity?] [message?] payload
    ```

    Example usage:

    ```
    $ cat text.txt | ws --rules rules.txt --ext ext2/*.c
    ```

  - Be extensible.

## Dependencies

hyperscan

## LICENSE

MIT
