# unity_metadata_loader
https://github.com/nevermoe/unity_metadata_loader を書き直したものです.

## 変更点
### unity_decoder
* windows以外でもコンパイルできるようになった.
* メタデータの取り出しに上限がなくなった. (MAX_META_COUNTが必要なくなった)

### unity_loader
* idcで書かれているのでIDA Pro以外でも動く. (若干文字列の命名の部分の挙動が違う)
* Android 32bitのlibil2cpp.soのみに対応している.

## How to Build
どこからか libil2cpp を取ってくる.
```bash
g++ -std=c++17 -I/path/to/libil2cpp/include unity_decoder.cpp -o unity_decoder
```
