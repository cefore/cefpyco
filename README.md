<!--
Copyright (c) 2018, National Institute of Information and Communications
Technology (NICT). All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the NICT nor the names of its contributors may be
used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
-->

# Cefore Python Compact パッケージ: cefpyco

- [Cefore Python Compact パッケージ: cefpyco](#cefore-python-compact-%e3%83%91%e3%83%83%e3%82%b1%e3%83%bc%e3%82%b8-cefpyco)
  - [概要](#%e6%a6%82%e8%a6%81)
  - [インストール方法](#%e3%82%a4%e3%83%b3%e3%82%b9%e3%83%88%e3%83%bc%e3%83%ab%e6%96%b9%e6%b3%95)
    - [Ceforeのインストール](#cefore%e3%81%ae%e3%82%a4%e3%83%b3%e3%82%b9%e3%83%88%e3%83%bc%e3%83%ab)
    - [cefpycoのインストール](#cefpyco%e3%81%ae%e3%82%a4%e3%83%b3%e3%82%b9%e3%83%88%e3%83%bc%e3%83%ab)
    - [cefpycoのアンインストール](#cefpyco%e3%81%ae%e3%82%a2%e3%83%b3%e3%82%a4%e3%83%b3%e3%82%b9%e3%83%88%e3%83%bc%e3%83%ab)
  - [構成](#%e6%a7%8b%e6%88%90)
    - [cefpyco](#cefpyco)
    - [cefapp](#cefapp)
    - [c_src](#csrc)
    - [test](#test)
  - [cefpycoの使用方法](#cefpyco%e3%81%ae%e4%bd%bf%e7%94%a8%e6%96%b9%e6%b3%95)
    - [cefnetdへの接続と切断](#cefnetd%e3%81%b8%e3%81%ae%e6%8e%a5%e7%b6%9a%e3%81%a8%e5%88%87%e6%96%ad)
    - [Interestパケットの送信](#interest%e3%83%91%e3%82%b1%e3%83%83%e3%83%88%e3%81%ae%e9%80%81%e4%bf%a1)
    - [Dataパケットの送信](#data%e3%83%91%e3%82%b1%e3%83%83%e3%83%88%e3%81%ae%e9%80%81%e4%bf%a1)
    - [パケットの受信](#%e3%83%91%e3%82%b1%e3%83%83%e3%83%88%e3%81%ae%e5%8f%97%e4%bf%a1)
    - [Publisherアプリの作成](#publisher%e3%82%a2%e3%83%97%e3%83%aa%e3%81%ae%e4%bd%9c%e6%88%90)
    - [Consumerアプリの作成](#consumer%e3%82%a2%e3%83%97%e3%83%aa%e3%81%ae%e4%bd%9c%e6%88%90)
  - [CefAppの使用方法](#cefapp%e3%81%ae%e4%bd%bf%e7%94%a8%e6%96%b9%e6%b3%95)
    - [概要](#%e6%a6%82%e8%a6%81-1)
    - [cefappconsumer.py](#cefappconsumerpy)
    - [cefappproducer.py](#cefappproducerpy)
    - [通信例](#%e9%80%9a%e4%bf%a1%e4%be%8b)
  - [注意事項](#%e6%b3%a8%e6%84%8f%e4%ba%8b%e9%a0%85)
  - [更新履歴](#%e6%9b%b4%e6%96%b0%e5%b1%a5%e6%ad%b4)
  - [Credit](#credit)

## 概要

cefpycoはceforeアプリ用pythonパッケージである。
以下のような特徴を持つ。

* もともとC言語で記述されているceforeの機能をpythonから呼び出せる。
  * pythonを用いてC言語よりも簡単に開発できる。
* InterestやDataの送受信が関数1つで実行できる。
* withブロックをサポートしており、cefnetdとの接続の初期化・終了処理を簡明に記述できる。
* 現在のceforeでは提供されていない「受信したInterestに応答してDataを返送するアプリケーション」の開発が容易にできる。
  * 言い換えれば、csmgrdを介さずにコンテンツを提供できる。
* (0.3.0以降) Optional TLVに対応(チャンク番号・終端チャンク番号・最大ホップ数・Interestライフタイム・コンテンツ期限・キャッシュ期限)。
  * この修正のため、0.3.0以前と以降で、CefpycoHandle.send_data()の引数の順序が変わっているので注意。

## インストール方法

### Ceforeのインストール

まず、PCにCeforeをインストールする(対象バージョン:0.8.1以降)。
CeforeのインストールはCeforeのサイト([https://cefore.net](https://cefore.net))から行える
（[Instruction](https://cefore.net/instruction) からユーザマニュアル、  [Download](https://cefore.net/download) から ソースコードを入手可能）。

### cefpycoのインストール

Ceforeおよびpythonがインストールされている環境で、以下のコマンドを実行する
(cmakeは"."をつけ忘れないように注意)。

```
sudo apt-get install cmake python-pip
pip install setuptools click numpy
cmake .
sudo make install
```

以降、任意のディレクトリから`import cefpyco`が可能となる。

pythonパッケージについて、setuptoolsはインストールに必須だが、
clickとnumpyはcefappを使わないならば不要。

### cefpycoのアンインストール

アンインストール時には`sudo make uninstall`を実行すればよい。

## 構成

以下のファイルおよびディレクトリから構成される。

* `README.md(.html)`: 本文書ファイル。
* `LICENSE.md(.html)`: ライセンスファイル。
* `CmakeLists.txt`: cmake用設定ファイル。
* `cefpyco/`: cefpycoのPythonパッケージ本体。
* `cefapp/`: cefpycoを用いたパイプライン通信可能なアプリケーション例。
* `c_src/`: Cefore機能呼び出しのためのC言語コード。
* `test/`: テスト用コード。
* `setup.py`: cefpycoインストール用設定の書かれたファイル。

以下では、各ディレクトリ内のファイルの説明を行う。

### cefpyco

* `__init__.py`: cefpycoパッケージのimport時に実行されるファイル。
* `core.py`: cefpycoの主要機能の書かれたファイル。

### cefapp

* `cefapp.py`: cefpycoを用いたパイプライン通信が可能なアプリケーション用クラスCefAppを提供。
* `cefappproducer.py`: CefAppを用いたConsumerアプリケーション。
* `cefappconsumer.py`: CefAppを用いたProducerアプリケーション。
* `test/test_cefapp.py`: cefappのテスト用ファイル。

<!-- 
開発者向け情報: 
test_cefapp.pyはpytestを利用している。
cefappディレクトリまたはcefapp/testディレクトリ下で`pytest`を実行すれば機能をテスト可能。
mockを使用しているため、cefnetdが起動していなくても機能をテストできる。
-->

### c_src

* `pywrap_cefpyco.c`: CeforeのPythonラッパー。cefpyco.cの関数をpythonで使用できるようにする。
* `cefpyco.c(.h)`: Ceforeの機能をラップして単純化した関数を提供。
* `cefpyco_parse.c(.h)`: cef_client_read関数で受信したTLV等を解析するための関数を提供。
* `cefpyco_def.h`: 各種定数等の定義。
* `cefpyco_util.c(.h)`: 便利関数群。
* `cefparse/cpcparse_tlv.c(.h)`: TLVヘッダ解析用関数を提供。
* `cefparse/cpcparse_type.c(.h)`: 解析途中の情報を保持するcpcparse_parse_info構造体および関連関数を定義。
    - cef_client_read関数から得たバッファを解析する関数をtry_parse型関数で定義している。
* `cefparse/cpcparse_app_frame_7_5.c(.h)`: 受信したCeforeアプリ用データ(`struct cef_app_frame`)を解析する関数を提供。Dataパケット受信に利用。Cefore 0.7.5以降用。
* `cefparse/cpcparse_interest.c(.h)`: 受信したInterestパケットを解析する関数を提供。Cefore 0.7.5以降用。
* `cefparse/cpcparse_app_frame.c(.h)`: 受信したCeforeアプリ用データ(`struct cef_app_frame`)を解析する関数を提供。Dataパケット受信に利用していた。Cefore 0.7.5以前のもの。
* `cefparse/cpcparse_interest.c(.h)`: 受信したInterestパケットを解析する関数を提供していた。Cefore 0.7.5以前のもの。
* `test/test_cefpyco.c`: cefpyco.cの自動テスト用コード。cefnetdが起動している状態で実行するとテストを実行できる。
* `test/run_cefpyco.c`: cefpyco.cの手動テスト用コード。引数の数に応じて5つの通信テスト用関数を実行できる。

また、各ディレクトリにcmake用の設定ファイルとして`CMakeLists.txt`が存在する。

### test

* `cefpycotest.py`: 共有ライブラリlibcefpyco.soの機能の手動テスト用のコード。
* `test_libcefpycotest.py`: 共有ライブラリlibcefpyco.soの機能の自動テスト用のコード。
* `test_core.py`: cefpyco/core.pyの自動テスト用のコード。

<!-- 
開発者向け情報: 
libcefpyco.soは、親ディレクトリ下で`cmake .`の代わりに`cmake -Dtest=on`を実行後、
`make`を実行することで得られる(インストール後に生成されるbuildディレクトリ下にも存在する)。
idを指定することで、c_src/test/test_cefpyco.cと同様に5つの通信テスト用関数を実行できる。
pytestの自動テストスクリプトではなく、手動で実行し、正常に動作しているか確認するためのもの。
-->

## cefpycoの使用方法

以下では、cefpycoを用いてcefnetdと通信する方法を説明する。

### cefnetdへの接続と切断

**接続から切断までの処理は、`cefpyco.create_handle()`を用いて簡単に実装できる。**

```python
import cefpyco

with cefpyco.create_handle() as handle:
    # Write code.
    pass
```

handleはCefpycoHandleというクラスのインスタンスとなっており、これを通じて通信を行う。
ファイル処理をwithブロックで行う場合と同様に、
withブロックの開始時にCeforeとの接続初期化処理が行われ、
withブロックの終了時に接続終了処理が行われる。
使用者がwithブロック内で変数handleを用いる限り、Ceforeの初期化・終了処理に悩む必要はなく、
アプリの処理を書くことに集中できる。

`create_handle()`関数は以下のオプション引数を持つ。

* enable_log(デフォルト値: True): cefpycoのログを表示するか否かを設定する。
* ceforedir(デフォルト値: None): cefnetd.conf等のceforeの設定ファイルのあるディレクトリを指定する。
    - Noneを指定した場合、環境変数`CEFORE_DIR`の値を用いる。
    - 環境変数も空の場合は、デフォルトのインストール先`/usr/local/cefore`を用いる。
* portnum(デフォルト値: 9896): アプリが接続するcefnetdのポート番号。

注意点として、withブロックを使った場合、そのブロックの外でhandleを使用することはできない。
また、ファイル処理とは異なり、
**withブロック内で再度`cefpyco.create_handle()`を実行することはできない**(シングルトンパターン)。

withブロックを使用したくない場合は、以下のようにCefpycoHandleインスタンスを生成し、
使用開始までに`begin()`メソッドを呼び出し、
使用終了後に`end()`メソッドを呼び出す。

```python
import cefpyco

handle = cefpyco.CefpycoHandle()
handle.begin()
# Write code
handle.end()
```

### Interestパケットの送信

次に、Interestパケットを送出する処理の実装例を示す。

```python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_interest("ccn:/test", 0)
```

**`h.send_interest(name, chunk_num)`を用いて、
コンテンツの名前`name`とチャンク番号`chunk_num`を指定して
Interestパケットを送出できる。**

複数のチャンクを受け取りたい場合は、複数のInterestを送出する必要がある。
例えば、100個のチャンクを受け取りたい場合は以下のようにする。

```python
import cefpyco

with cefpyco.create_handle() as h:
    for i in range(100):
        h.send_interest("ccn:/test", i)
```

注意点として、これはInterestパケットを送出するだけであり、
Dataパケットを受信するためには後述の「パケットの受信」を用いる必要がある。

また、以下のキーワード引数を持つ。

* `hop_limit`: Interestの最大ホップ数。
* `lifetime`: Interestのライフタイム(ミリ秒で指定)。

また、通常のInterestではなく、Symbolic Interest (SMI)を用いたい場合は、
**`h.send_symbolic_interest(name)`**を用いる。
SMIは以下の2つの特徴を持つ。
(1)cefnetdがDataを受信してもPITエントリが消えず、タイムアウトによってのみ削除される。
(2)チャンク番号無しのprefixを指定すると、チャンク番号付きの任意のDataを受信できる
(例:SMIで`ccn:/a`を要求すると、`ccn:/a/Chunk=0`, `ccn:/a/Chunk=1`, ... 等の名前のDataすべてを受け取れる)。
主に動画のリアルタイムストリーミング目的で用いるためのもの。

### Dataパケットの送信

Dataパケットを送出する処理の実装例は以下の通りである。

```python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_data("ccn:/test", "hello", 0)
```

**`h.send_data(name, payload, chunk_num)`メソッドは、
コンテンツの名前`name`とチャンク番号`chunk_num`に加えて、
payload(cobのデータ)をstring型で指定する必要がある**
(高速化のためデータの型チェックを省いており、string型でないデータの動作は保証しない)。

Interestパケットと同様、
これはDataパケットを送出するだけのメソッドであり、
Interestパケットを受信してから応答するためには後述の「パケットの受信」を用いる必要がある。

また、以下のキーワード引数を持つ。

* `end_chunk_num`: コンテンツの最後のチャンク番号。
* `hop_limit`: 最大ホップ数。
* `expiry`: コンテンツの有効期限。現在からの相対時間をミリ秒で指定する。
* `cache_time`: コンテンツの推奨キャッシュ時間。現在からの相対時間をミリ秒で指定する。

### パケットの受信

**パケットを受信するメソッドは`h.receive()`であり、
CcnPacketInfoオブジェクトを返す。**

以下は、Interestを送信後、Dataを待ち受け、最後に受信したデータを表示する処理の実装例である。

```python
import cefpyco

with cefpyco.create_handle() as h:
    h.send_interest("ccn:/test", 0)
    info = h.receive()
    print("info.is_succeeded:{0}".format(info.is_succeeded))
    print("info.is_failed:{0}".format(info.is_failed))
    print("info.version:{0}".format(info.version))
    print("info.type:{0}".format(info.type))
    print("info.actual_data_len:{0}".format(info.actual_data_len))
    print("info.name:{0}".format(info.name))
    print("info.name_len:{0}".format(info.name_len))
    print("info.chunk_num:{0}".format(info.chunk_num))
    print("info.end_chunk_num:{0}".format(info.end_chunk_num))
    print("info.payload:{0}".format(info.payload))
    print("info.payload_s:{0}".format(info.payload_s))
    print("info.payload_len:{0}".format(info.payload_len))
```

**receiveメソッドはオプション引数`error_on_timeout`(デフォルト値: False)を持つメソッドである。
メソッド実行後約4秒間待機し、受信できなかった場合は処理を戻す。**
成功するまで受信したい場合は、while等のループ構文を使用する必要がある。
この際、error_on_timeoutをTrueに指定すると、
CefpycoHandleはCeforeとの接続を切断する
(そのため、もう一度receiveしようとしても失敗する)。

receiveメソッドは、
受信したパケットがInterestかDataかに関わらず、
受信結果をCcnPacketInfoオブジェクトに入れて返す。
このオブジェクトの各プロパティ値の意味は以下の通りである。

* is_succeeded/is_failed: 受信成功/失敗フラグ。
* version: 受信したパケットのバージョン値。
* type: 受信したパケットのタイプ値。DataかInterestかを見分けることが可能。
* actual_data_len: 受信したパケット（またはデータ）のヘッダを含むバイト長。
* name: string型で表された名前。
* name_len: 名前の長さ。
* chunk_num: チャンクの数。
* end_chunk_num: コンテンツの最後のチャンク番号(Data送出時に設定したときのみ)。
* payload: ペイロードのバイト列(空の場合は`(empty)`と表示される)。Python2ではstr、Python3ではbytesを返す。
* payload_s: stringで表されたペイロード(空の場合は`(empty)`と表示される)。Python2ではunicode、Python3ではstrを返す。
* payload_len: ペイロードの長さ。空の場合は0となる。

また、`is_interest`および`is_data`プロパティを持つ。
しかし、後述のようにInterestとDataでは取得する方法が異なるため、
InterestとDataを同時に受信するようなアプリケーションでなければ、
受信したパケットがInterestかDataを区別する必要は無いだろう。

Interest を受信した場合に、それが SMI であるかを判別するには、
`is_symbolic` プロパティを利用できる。
特に Interest であるかと SMI か否かを同時に判別したい場合は、
通常の Interest (Regular Interest) については `is_regular_interest` プロパティ、
SMI については `is_symbolic_interest` プロパティを利用できる。

注意点を以下に述べる。

* receiveメソッドは、cefnetdが受信したすべてのパケットを待ち受ける訳ではない。
    - Interestパケット受信のためにはregisterメソッドを呼び出す必要がある。
    - Dataパケット受信のためにはInterestパケットを先に送信する必要がある。
* あまりに多くのパケットをcefnetdのバッファに貯めておくことはできないため、
    適当な間隔でreceiveメソッドを呼び出して、
    アプリがcefnetdからパケットを受け取るような実装にすることが推奨される。
* DataパケットはCeforeのアプリ用に用意された構造体に収めてアプリに送られるため、
    `actual_data_len`はCCNxパケットフォーマットの仕様から期待される長さと一致するとは限らない。

### Publisherアプリの作成

今までの機能を利用して、pubilsherアプリ
(Interestを待ち受け、その後Dataパケットを返送するアプリ)を
実装した例を以下に示す。

```python
import cefpyco

with cefpyco.create_handle() as h:
    h.register("ccn:/test")
    while True:
        info = h.receive()
        if info.is_success and info.name == "ccn:/test" and info.chunk_num == 0:
            h.send_data("ccn:/test", "hello", 0)
            # break # Uncomment if publisher provides content once
```

**Interestの待ち受けには、`h.register()`メソッドを用いる。**
これは、自分が受け取りたいInterestのプレフィックスをcefnetdに登録するメソッドである。
例えば、`ccn:/test`の他に、`ccn:/test/foo/bar.txt`のような名前のInterestも受け取ることができる。

その後、`h.receive()`を実行し続ける。
対象のInterestの受信に成功した場合は、
`h.send_data()`を用いてDataパケットを返送する。
publisherアプリなので無限ループし続けるが、
一度しかコンテンツを提供しない場合はbreak等でループを抜ければ良い。

### Consumerアプリの作成

更に、consumerアプリ(Interestを送出後、Dataを待ち受けるアプリ)を
実装した例を以下に示す。

```python
from time import sleep
import cefpyco

with cefpyco.create_handle() as h:
    while True:
        h.send_interest("ccn:/test", 0)
        info = h.receive()
        if info.is_success and info.name == "ccn:/test" and info.chunk_num == 0:
            print("Success")
            print(info)
            break # Uncomment if publisher provides content once
        sleep(1)
```

**Interestを送出すれば、その後receiveメソッドでDataを受信できる。**
受信に成功した場合は`Success`と表示して終了する。

この実装例ではDataを受信できるまでループしているが、
Interestの送出間隔が短いとPITに集約されてしまうため、
ループのたびに`sleep(1)`を挟んでいる。

この例では1つのチャンクを受け取ったらループを抜けているが、
チャンク番号を変えながらループすれば、
複数チャンクから成るコンテンツを受け取ることも可能となる。
多数のチャンクから成るコンテンツを受信したい場合は、
一度に複数のInterestを送ることで高速通信も可能である。
しかし、その際は同時に送出するInterestの数に注意(cefnetd.confのPITサイズや処理性能の制約等)し、
一度に送りきれない場合はパイプライン処理を実装することを推奨する。

## CefAppの使用方法

以下では、CefpycoHandleを用いて実装されたコンテンツ要求/提供ツールである、
cefappconsumerツールとcefappproducerツールについて説明する。

### 概要

cefappconsumerツールとcefappproducerツールは、以下の特徴を持つ
（以下ではconsumer/producerと略記）。

* consumerはInterestを送出してDataを受信し、producerはInterestを待ち受けてDataを送信する。
* 入出力はインライン・標準入出力・ファイルの3種類を選択できる。
* consumerはパイプライン処理を実装しており、producerと高速通信が可能。
    - 参考値として、開発者の環境における簡単な実験では約80Mbpsを観測。

2つのツールは対となるものであり、
cefnetd起動後に、consumerとproducerを起動すると、
consumerがproducerからコンテンツを取得するための通信が行われる。
consumerとproducerの起動順は問わないが、
どちらかがタイムアウトしてしまうまでにはもう片方を起動しなければならない
(デフォルトのタイムアウト設定では約8秒以内)。

CefApp特有の処理として、consumer/producer間でコンテンツのcob数情報を共有するために、
consumerがproducerに対してコンテンツ名の最後に`/meta`を付けたコンテンツを要求し、
cob数を取得する処理が事前に行われる。
そのため、少なくともconsumerに関しては、
他のData提供ツール(cefputfile等)と組み合わせて用いることができないので注意
(cefappproducerとcefgetfileの組み合わせは通信可能)。

直接コンテンツの取得・提供の目的で用いるよりは、
開発者がCefpycoHandleを用いたプログラム作成の参考にすることを想定して作成されている。

### cefappconsumer.py

* 使用方法
    ```
    cefappconsumer.py [OPTIONS] name
    ```
* 概要
    - nameで指定したコンテンツ名のDataを要求し、受信したコンテンツの内容を出力する。
* オプション
    - `[-t|--timeout int]`: ConsumerがDataの待ち受け失敗を許容する回数を指定する。１回の待ち受け処理で約4秒間待ち受ける。デフォルトは2回。
    - `[-p|--pipeline int]`: パイプライン数。多すぎるとPITがあふれる・cefnetdの処理限界を超える等の現象が起きうるので、cefnetd.confのPITサイズや処理性能の制約等に要注意。デフォルトは10。
    - `[-f|--filename str]`: fileモード(`-o`オプション参照)使用時に使用するファイル名を指定する。また、`-o`オプションで明示的にfileモードにせずとも、ここでファイル名を指定した場合はfileモードとして扱われる。デフォルトではnameの最後のセグメント名が用いられる。
    - `[-o|--output mode]`: 出力モードを指定する。modeには以下のいずれかの文字列を指定する。デフォルトはstdoutモード。
        - none: 無出力モード。どこにもデータを出力しない(内部のバッファにも貯めないので軽量)。
        - stdout: 標準出力モード。端末等の標準出力に対して受信したDataの内容を出力する。
        - file: ファイル出力モード。nameの最後のセグメント名、もしくはfilenameで指定した名前の
            ファイルに、受信したコンテンツの内容を出力する。
    - `[-q|--quiet]`: 指定した場合、ログの出力を行わない。
* 使用例
    - `./cefappconsumer.py ccn:/test`
        - ccn:/testという名前のコンテンツを受信し、標準出力に受信したコンテンツを出力する。
    - `./cefappconsumer.py ccn:/test/a -o file`
        - ccn:/test/aという名前のコンテンツを受信し、ファイル名`a`に受信したコンテンツを出力する。
    - `./cefappconsumer.py ccn:/test/a -o file -f b`
        - ccn:/test/aという名前のコンテンツを受信し、ファイル名`b`に受信したコンテンツを出力する。
    - `./cefappconsumer.py ccn:/test -o none -q`
        - ccn:/testを受信するための通信のみ行い、ログや受信したコンテンツを一切出力しない。

### cefappproducer.py

* 使用方法
    ```
    cefappproducer.py [OPTIONS] name [arg]
    ```
* 概要
    - arg等で指定したコンテンツからDataを作成し、nameで指定したコンテンツ名でDataを提供する。
* オプション
    - `[-t|--timeout int]`: ProducerがDataの待ち受け失敗を許容する回数を指定する。１回の待ち受け処理で約4秒間待ち受ける。デフォルトは2回。
    - `[-b|--block_size int]`: 1つのcobのサイズ。デフォルトは1024バイト。最小値・最大値の制約をつけていないため、MTUに注意して使用すること。
    - `[-i|--input]`: 入力モードを指定する。modeには以下のいずれかの文字列を指定する。デフォルトはargモード。
        - arg: インラインモード。引数argに直接コンテンツ内容を記述する。
        - stdin: 標準入力モード。標準入力の内容からコンテンツを作成する。
        - file: ファイル入力モード。nameの最後のセグメント名、もしくは引数argに指定したファイル名のファイルからコンテンツを作成する。
    - `[-q|--quiet]`: 指定した場合、ログの出力を行わない。
* 使用例
    - `./cefappproducer.py ccn:/test helloworld`
        - `hellowolrd`という10文字のコンテンツをccn:/testという名前で作成し、提供する。
    - `./cefappproducer.py ccn:/test helloworld -b 5`
        - `hello`と`world`という5文字ごとのコンテンツをccn:/testという名前で作成し、提供する
            (`hello`はチャンク番号0,`world`はチャンク番号1として提供される)。
    - `./cefappproducer.py ccn:/test/a -i file`
        - `a`という名前のファイルからコンテンツを作成し、ccn:/test/aという名前で提供する。
    - `./cefappproducer.py ccn:/test/a b -i file -o none -q`
        - `b`という名前のファイルからコンテンツを作成し、ccn:/test/aという名前で提供する。

### 通信例

cefappproducerが"hello"という文字列を`ccn:/test`という名前で発行し、
それをcefappconsumerが取りに行く通信例を示す。

以下では、cefnetdを起動した状態で、
cefappproducerを端末t1、
cefappconsumerを端末t2で起動する
(尚、各ツールで指定可能なオプションは`--help`オプションで確認できる)。

```
t1$ sudo ./cefappproducer.py ccn:/test hello
[cefpyco] Configure directory is /usr/local/cefore
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
[cefapp] Receiving Interest...
t2$ sudo ./cefappconsumer.py ccn:/test
[cefpyco] Configure directory is /usr/local/cefore
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Config directory is /usr/local/cefore
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Local Socket Name is /tmp/cef_9896.0
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: [client] Listen Port is 9896
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccn:/test/meta, #chunk: 0)
YYYY-MM-DD hh:mm:ss.xxx [cefpyco] INFO: Send interest (name: ccn:/test, #chunk: 0)
[cefapp] Succeed to receive.
hello
```

## 注意事項

* 本ツールはCeforeを用いたアプリ開発を簡易化することを目的に個人的に開発されたが、デバッグが不十分。
    - Ubuntu 16.04とRaspberry piでのみ動作確認済み(cefpyco 0.2.0現在)。
    - python3での動作確認のみを行っている。
* cefnetdをsudo権限で起動する場合、pythonツール使用時にもsudoが必要なことがある。
    ツールの起動に失敗する場合は、起動時のログを確認して`Permission Denied`などと出ていないか確認し、
    必要に応じてsudo権限で実行すること。
* cefnetdをcefnetdstopを使わずにkillなどで強制終了させた場合、
    `Already another cefnetd is using the same listen port`と出てその後の起動に失敗する場合がある。
    * 原因はソケットファイルが`/tmp`に残存してしまうため。
    * この場合は`sudo cefnetdstop -F`を実行すると、ソケットファイル削除を含む強制終了処理が実行されるので、それを試す。
* ~~アプリがInterestを受け取れない場合は、FIBにマッチするエントリが無いか確認すること。~~
    - ~~特にFIBに`ccn:/`宛のエントリが登録されている場合等、そちらが優先される場合がある(cefore 0.7.2現在)。~~
    - Cefore 0.8.0現在、アプリがInterestを優先して取得するように仕様変更された。
* 通信に失敗する場合は、cefnetd起動前にバッファチューニングを行っているか確認すること。
    - Linux OSの場合は以下を実行。
    ```
    $ _cef_mem_size=10000000
    $ sudo sysctl -w net.core.rmem_default=$_cef_mem_size
    $ sudo sysctl -w net.core.wmem_default=$_cef_mem_size
    $ sudo sysctl -w net.core.rmem_max=$_cef_mem_size
    $ sudo sysctl -w net.core.wmem_max=$_cef_mem_size
    ```
    - Mac OSの場合は以下を実行。
    ```
    $ sudo sysctl -w net.local.stream.sendspace=2000000
    $ sudo sysctl -w net.local.stream.recvspace=2000000
    ```
    - 特に高速通信・大容量ファイルの通信を行う場合は、バッファを大きくすることを推奨する。
    - PCの再起動の度にこの値は初期化されるので、PC起動時の常時実行に入れる等、実行忘れ防止策を取ることを推奨。
* Interestの生存期間は4秒、Dataのキャッシュ生存時間は10時間に設定している。
    - 変更する場合はcefpyco.cの該当箇所の値を変更すること。
* cmake実行後、ディレクトリごと別の場所にコピーして再度cmakeを実行しようとすると失敗する。
    - 再度cmakeする場合は、CMakeFilesディレクトリ・CMakeCache.txt・cmake_install.cmakeを手動で削除する必要がある。
    - `cmake clean`などのコマンドは存在しない。

## 更新履歴

* 2021/02/25: Symbolic Interest (SMI) の送受信に対応。
* 2019/08/21: バイナリデータの送受信に対応。CcnPacketInfo.payloadのデフォルトをバイト列に変更し、文字列用にpayload_sを追加。
* 2019/08/06: Cefore-0.8.1に対応。Mac環境用に微調整。
* 2019/05/15: Cefore-0.8.0に対応。テストコードを追加。Optionalヘッダ等の機能を追加。CefpycoHandle.send_dataの引数順が変更されたので要注意。
* 2018/08/09: ceftoolからcefpycoに名称を変更。cefore-0.7.2に対応。setuptoolsを用いたインストールに対応。ファイル構造の大幅な修正を反映。
* 2018/07/30: CefAppの使用方法を追記。ファイル構造の変更を反映。その他微修正。
* 2018/07/23: 初版作成(cefore-0.7.1用)。

## Credit

* 大岡 睦 (Atsushi Ooka), National Institute of Information and Communications Technology (NICT), a-ooka@nict.go.jp
