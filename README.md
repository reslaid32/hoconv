# hoconv

## Description

The **hoconv** project is a universal data format converter with serialization and deserialization support, built on the [hoserial](https://github.com/reslaid32/hoserial.git) library.

It allows you to conveniently convert data between JSON, YAML, TOML, and other formats — for example, easily doing `json2yaml`, `json2toml`, etc. — using a single common driver.

---

## Dependencies

The header-only library **hoserial** is used for operation.

### Initializing dependencies

```bash
git submodule update --init --recursive
```

### Updating dependencies

```bash
git submodule update --remote --merge
```

---

## Build and Install

```bash
make
sudo make install
```

## Usage

An example command to convert the file `example.json` from JSON to YAML and save the result in `example.yaml`:

```bash
hoconv --input-type json --file example.json --output-type yaml --output example.yaml
```

## Additional information

- Supported formats: `json`, `yaml`, `toml`
- You can specify the format of the input and output files using the `--input-type` (`-it`) and `--output-type` (`-ot`) options
- By default, if no file is specified for reading, the input data is read from `stdin`
- Help and version options: `--help` (`-h`), `--version` (`-v`)