# zephyr-esp32s3-advent2026

Qiita Advent Calendar 2026「Zephyr RTOS」シリーズのサンプルコードです。ESP32-S3 のボードで Zephyr RTOS を動かします。

- Qiita 記事（公開予定）: ESP32 で Zephyr RTOS を動かす環境構築と Lチカ

シリーズの続きの記事（Device Tree、Kconfig、Sensor API、West と CI）のコードも、このリポジトリに追加していく予定です。

## 動作を確認した環境

| 項目 | 内容 |
| --- | --- |
| ボード | ESPr® Developer S3 Type-C（スイッチサイエンス、ESP32-S3-WROOM-1-N16R8） |
| Zephyr のボード名 | `esp32s3_devkitc/esp32s3/procpu`（専用の定義が無いので ESP32-S3-DevKitC の定義を流用） |
| Zephyr | v4.5.0 |
| Zephyr SDK | 1.0.1（ツールチェイン `xtensa-espressif_esp32s3_zephyr-elf`） |
| PC | Windows 11、PowerShell |

## 中身

| フォルダ | 内容 | 記事 |
| --- | --- | --- |
| `apps/blinky-ext` | Zephyr の `samples/basic/blinky` に、外付け LED を `led0` として定義する `app.overlay` を足したもの | 環境構築と Lチカ |
| `apps/dt-overlay` | Device Tree の overlay の例。外付け LED を `led0` に、I2C0 を有効にして 400kHz に、I2C0 のピンを GPIO8/9 に付け替え（`i2c0-gpio8-9.overlay`）。起動時に I2C バスのアドレスを探して表示する | Device Tree |

### apps/blinky-ext の配線

| ボード | 接続先 |
| --- | --- |
| IO7 | 抵抗（330Ω など）→ LED のアノード |
| GND | LED のカソード |

LED は1秒ごとに点滅し、シリアルに `LED state: ON` / `OFF` が出ます。

## ビルドと書き込み

Zephyr v4.5.0 のワークスペースと Zephyr SDK 1.0.1 を、公式の Getting Started Guide の手順で用意しておきます。ESP32 用のバイナリ blob も必要です（`west blobs fetch hal_espressif`）。

- Getting Started Guide: https://docs.zephyrproject.org/4.5.0/develop/getting_started/index.html

このリポジトリをワークスペースの外に置く場合は、Zephyr の venv を有効にして、環境変数 `ZEPHYR_BASE` にワークスペースの `zephyr` フォルダを指定します。

```powershell
# 例: ワークスペースが <ワークスペース> にある場合
<ワークスペース>\.venv\Scripts\Activate.ps1
$env:ZEPHYR_BASE = '<ワークスペース>\zephyr'

cd zephyr-esp32s3-advent2026
west build -p always -b esp32s3_devkitc/esp32s3/procpu -S espressif-flash-16M apps/blinky-ext
west flash --esp-device COMx
west espressif monitor -p COMx   # 終了は Ctrl+]
```

- `COMx` は、ボードをつないだときにデバイスマネージャーに出る COM ポートに置き換えてください。
- `-S espressif-flash-16M` は、ボードのフラッシュ（16MB）に合わせる snippet です。DevKitC の定義は 8MB 前提なので、付けなくても動きます（起動ログにフラッシュの大きさの警告が出ます）。
- `-S` は west の引数なので、`--` より前に書きます。

## ライセンス

Apache License 2.0（[LICENSE](LICENSE)）。`apps/blinky-ext` のソースは Zephyr の `samples/basic/blinky`（Apache-2.0）を元にしています。
