KEY = b"FORMLESS_WILL_REMEMBER_YOU"

import pathlib

def xor_bytes(data: bytes, key: bytes) -> bytes:
    out = bytearray(len(data))
    for i, b in enumerate(data):
        out[i] = b ^ key[i % len(key)]
    return bytes(out)

def encrypt_file(path: pathlib.Path):
    raw = path.read_bytes()
    enc = xor_bytes(raw, KEY)
    path.with_suffix(path.suffix + ".enc").write_bytes(enc)

def main():
    for path in pathlib.Path("assets").rglob("*.txt"):
        encrypt_file(path)

    for path in pathlib.Path("levels").rglob("*.txt"):
        encrypt_file(path)

if __name__ == "__main__":
    main()
