# zephyr-esp32s3-advent2026

[![Build](https://github.com/ramtuc/zephyr-esp32s3-advent2026/actions/workflows/build.yml/badge.svg)](https://github.com/ramtuc/zephyr-esp32s3-advent2026/actions/workflows/build.yml)

Qiita Advent Calendar 2026「Zephyr RTOS」シリーズのサンプルコードです。ESP32-S3 のボードで Zephyr RTOS を動かします。

- Qiita 記事（公開予定）: ESP32 で Zephyr RTOS を動かす環境構築と Lチカ

シリーズの記事（環境構築、Device Tree、Kconfig、Sensor API、West と CI）のコードを、このリポジトリにまとめています。

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
| `apps/dt-overlay` | Device Tree の overlay の例。外付け LED を `led0` に、I2C0 を有効にして 400kHz に、I2C0 のピンを GPIO8/9 に付け替え（`i2c0-gpio8-9.overlay`）、BME280 を `bme280@76` として I2C0 に宣言。起動時に I2C バスのアドレスを探して表示する | Device Tree |
| `apps/kconfig-footprint` | Kconfig で ROM と RAM を削る例。LED を点滅させてログを出すだけのアプリを、`prj_0_full.conf`（シェル・ロギング・ネットワークと Wi-Fi）から `prj_5_no_console.conf`（ログもコンソールも無し）まで、設定だけを変えてビルドする。`debug_stacks.conf` はスタックの使用量を測るときに足す | Kconfig |
| `apps/sensor-fetch-get` | BME280 の温度・気圧・湿度を、センサ API の Fetch/Get（`sensor_sample_fetch` / `sensor_channel_get`）で2秒ごとに読む。センサは Device Tree の別名 `ambient-temp0` で取る | Sensor API |
| `apps/sensor-read-decode` | 同じ値を Read/Decode（`sensor_read` とデコーダ）で読む | Sensor API |
| `apps/sensor-shell` | sensor shell（`CONFIG_SENSOR_SHELL`）入りのビルド。`main` は何もせず、シリアルのシェルで `sensor get bme280@76` と打って値を見る | Sensor API |

### apps/blinky-ext の配線

| ボード | 接続先 |
| --- | --- |
| IO7 | 抵抗（330Ω など）→ LED のアノード |
| GND | LED のカソード |

LED は1秒ごとに点滅し、シリアルに `LED state: ON` / `OFF` が出ます。

### Sensor API のアプリの配線

`apps/dt-overlay` の BME280 と同じです。BME280 モジュール（秋月電子通商 AE-BME280）を I2C0 につなぎます。

| ボード | AE-BME280 |
| --- | --- |
| IO1 | SDI（SDA） |
| IO2 | SCK（SCL） |
| 3V3 | VDD、CSB（I2C モードにする） |
| GND | GND、SDO（I2C アドレスを 0x76 にする） |

`apps/sensor-fetch-get` と `apps/sensor-read-decode` の `sht4x.overlay` は、温度センサを SHT4x（Sensirion）に替える場合の例です。`-DEXTRA_DTC_OVERLAY_FILE=sht4x.overlay` を付けてビルドできることだけを確かめています（SHT4x の実機では動かしていません）。

## West のワークスペースとして使う

このリポジトリは、West の manifest（`west.yml`）を持っています。Zephyr は v4.5.0 に固定し、モジュールは `hal_espressif`、`mbedtls`、`tf-psa-crypto` だけを取ります。新しいフォルダで次のようにすると、このリポジトリ用のワークスペースができます（Zephyr SDK 1.0.1 は入っている前提）。

```powershell
mkdir C:\zephyr-ci
cd C:\zephyr-ci
git clone https://github.com/ramtuc/zephyr-esp32s3-advent2026
py -3.12 -m venv .venv
.venv\Scripts\Activate.ps1
pip install west
west init -l zephyr-esp32s3-advent2026
west update
west packages pip --install
west blobs fetch hal_espressif
```

### 全部のアプリをまとめてビルドする（twister）

各アプリの `sample.yaml` に、記事で使ったビルドの形（16 通り）を書いています。

```powershell
cd zephyr-esp32s3-advent2026
west twister -T apps -v --inline-logs --integration --short-build-path -O C:/zt
```

Windows では、パスの長さの制限を避けるために `--short-build-path` と短い出力先（`-O`）を付けます。

### GitHub Actions

`.github/workflows/build.yml` で、main への push と pull request のたびに次の3つを動かします。

| ジョブ | 内容 |
| --- | --- |
| build | twister で全部のアプリをビルドし、メモリの使用量を Summary に表で出す（`scripts/footprint_summary.py`） |
| sca | GCC の静的解析（`ZEPHYR_SCA_VARIANT=gcc`）を付けてビルドし、`apps/` のコードに指摘があれば失敗にする（`scripts/sca_report.py`） |
| qemu | `apps/blinky-ext` を `esp32s3_devkitc/esp32s3/procpu/qemu` 向けにビルドし、Espressif の QEMU で動かして `LED state: ON` が出るかを確かめる |

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

### apps/kconfig-footprint のビルド

設定ファイルは `-DCONF_FILE=` で選びます（PowerShell では `-D...` を `"` で囲みます）。

```powershell
west build -p always -b esp32s3_devkitc/esp32s3/procpu apps/kconfig-footprint -- "-DCONF_FILE=prj_4_stacks.conf"
west build -t rom_report
west build -t ram_report
```

`prj_0_full.conf` は Wi-Fi のドライバを含みます。大きさを比べるための構成で、記事では実機に書き込んでいません。

## ライセンス

Apache License 2.0（[LICENSE](LICENSE)）。`apps/blinky-ext` のソースは Zephyr の `samples/basic/blinky`（Apache-2.0）を元にしています。
