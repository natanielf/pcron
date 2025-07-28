# Pretty Printer for `cron` Expressions

## Build

```sh
make pcron
```

or

```sh
gcc -Wall pcron.c -o pcron
```

## Run

```sh
./pcron "* * * * * [COMMAND]"
```

> Note: `cron` expressions must be enclosed in quotes (either single or double)

## References

- https://en.wikipedia.org/wiki/Cron
