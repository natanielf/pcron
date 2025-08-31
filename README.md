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

## Example

```sh
./pcron "10,20,30 9-17 * JAN-JUN * ls"
```

## References

- https://en.wikipedia.org/wiki/Cron
