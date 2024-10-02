# ATL1200
```
ATL/1200マニュアル
11/10/2001作成，11/25/2001最終修正

■□概要□■
・AT90S1200専用の，アセンブラに近い簡単な言語のコンパイラ，
　およびラインエディタとライタのささやかな統合環境です。
　ATLとは，Avr Tiny Languageの略です。
　アセンブラよりもソースコードが短く，ポケコンで打ち込みやすいよう配慮しています。
　ポケコンを用いて，いつでもどこでも，気軽にプログラミングできます。
・なぜポケコン？パソコンで結構という方には必要ないプログラムです。
・基本的に１行１命令（＝１クロック）です。
・各行のはじめにはスペースを入れられます。
・行間に改行を入れられます。先頭にスペースがあってもかまいません。
・コメントは小文字でもかまいません。
・任意の場所に飛ぶGOTO文に相当するものはありません。実装する予定もありません。
・本プログラムはGPL3準拠のフリーソフトウェアです。

■□インストール□■
ATL1200p.cをPC-G850にインストールするには，
下記の接続ケーブルを用いてシリアルインターフェイスで送り込むか，
または，「打ち込み」ます。
シリアルインターフェイスの設定を母艦とポケコンで同じようにして下さい。
同じフォルダ内のATL1200.cは，パソコン上で評価するためのものです。
この際，エディタの挿入コマンドはリターンが効きませんので'I'を打ち込みます。

■□変数□■
・符号無し１バイト変数(unsigned char)です。
・GAME言語に習って大文字のA-Zです。それぞれレジスタ00h-19hに対応します。
・AT90S1200のハードウェア仕様から，以下の制限があります。
　1.グローバル変数です。
　2.定数を代入／条件比較できるのはQ-Zです。
　3.I/Oのビットセット／クリアが可能なのは00h-19hまでです。
　　（それ以上のアドレスへは，I/Oへのレジスタのand/or指定および代入で代用して下さい。）

■□定数□■
・単独の場合は'.'で始まります。
・１６進数／２進数のいずれかです。
　１６進数の場合は大文字の'H'で始まり，
　　２進数の場合は大文字の'B'で始まります。
・１６進数の場合は2桁の0-9,A-Fからなる数，
　　２進数の場合は8桁の0/1からなる数のみ受け付けます。
例）
A=.H1D
B=.B00010110
A=.H9はエラー。

■□I/O□■
・先頭が'['で始まり，その後には定数のみが入り，I/Oアドレスを表します。
例）
[H18
[B00101100　（多分使わないと思う）

■□ビット指定□■
I/Oまたは変数のビット指定は，その後に'.'をおき，その後に0-7の数字を指定します。
変数へのビット代入はできません。条件判断のみです。
例）
[H08.7=1
;A.2=1 #WAIT

■□メイン／サブルーチン／割り込みルーチンラベル□■
*/**[サブルーチン名]
・12文字以内，途中に空白を入れられます。
・次の'*'または'**'までがルーチンの本体で，その時点でリターンです。
・メインルーチンは一番後ろのルーチンに自動的に設定されます。
　リセット時に最初に実行されるルーチンとなります。
　この場合は，命令がコードの終端に達するとメインルーチンの先頭にジャンプします。
・ルーチンの数は20以内です。
・先頭が'**'であれば，３種類の割り込み全てでこのルーチンに飛びます。
　このルーチンは一つのみです。
・割り込みルーチンをその後のルーチンから呼ぶことは避けて下さい。
　
例）
*WAIT 
...
...
...　 -> 自動的にリターン
**INTR <-割り込みベクタからジャンプ
...
...
...　 -> 割り込みから復帰（RETI命令）
*MAIN <- リセット後にここにジャンプ
...     |（メインルーチン）
...     |
...     |
(EOF)---　自動的にメインルーチン先頭にジャンプ

■□サブルーチンコール□■
#[サブルーチン名]
・その位置より前で宣言されたルーチンである必要があります。
・自分自身を呼ぶ場合はルーチンの先頭にジャンプ(RJMP命令)を使用しますので，スタックを消費しません。
・サブルーチンコールの階層は３以内です。（AT90S1200のハードウェア仕様）
例）
#WAIT

■□条件分岐□■
;[条件式]

・';'の直後の１命令を評価し，真である場合は同じ行のその後の１命令を実行します。
・その後の文字はコメントとして扱われます。（無効）
・評価が偽である場合は次の行に進みます。

例）
;A=B [H18.0=1  :SET PORTB.0
;B<Z #WAIT   : WAIT 100msec

■□繰り返し処理：Do-Whileループ□■
(
...
)[条件式]

・'('でループの開始を指定し，その後ろはコメントとして扱われます。
・')'の後ろに条件判定式を置きます。
・条件式が満たされる間，'('と')'の間の命令を実行します。
・ループの入れ子の深さは10以内です。

例）
*VWAIT
R=[H18
Z=.HFF
(         :wait
  S=.HFF
  (
    T=.HFF
    (
      T--
    )T‾.H00
    S--
  )S‾.H00
  R--
)R>Z

■□条件式□■
以下のパターンがあります。
１：変数<-->変数
２：変数<-->定数（変数はQ..Z）
３：I/O.bit=1/0
それぞれ使用できる比較演算子は以下の通りです。
'‾'は「等しくない」を表します。
１：=,‾,>,<,<=,>=
２：=,‾
３：=

■□代入／演算□■
５つのパターンがあります。

０：変数（単項演算子）
１：変数<--変数
２：変数<--定数（変数はQ..Z）
３：I/O<--変数
４：変数<--I/O
５：I/O.bit<--ビット（00h-19hまで）

それぞれ使用できる演算子は以下の通りです。
０：!,++,--,>,<（それぞれnot／インクリメント／デクリメント／右シフト／左シフト）
　　単項演算子で，A!,A++,A>のように後置で使用します。
１：+=,-=,=,&,|（A&B,A|Bは，AにそれぞれA and B,A or Bを代入することを表します。）
２：=,&,|
３：=
４：３と同じ
５：=1または=0

■□起動時の環境□■
起動時には，エディタ／コンパイラ／ライタを使用するコマンド待ちになります。
L：ファイルのロード
S：ファイルのセーブ
C：コンパイル
W：AVRへのライト
2：１行上スクロール
8：１行下スクロール
E：該当行のエディット
D：該当行の削除
リターン：１行入力／挿入

注意：
・Wコマンドは，Cコマンドでコンパイルエラーを生じなくなった後に使用して下さい。
・Cコマンドは，各コマンド生成ごとに任意のキーを入力します。（ステップ実行）
・入力時，スペースは無視されます。スペースを入れたい場合は','（カンマ）で代用して下さい。
　例：",,,hello,world." -> "   hello world."
・該当行とは，画面一番下側に表示されている行のことです。
・エディット，削除，挿入時には画面左側にプロンプト':'があらわれます。

■□ファイル□■
PC-G850のRAMファイルは，プログラムからの書き込み時でも既にファイル名が存在している必要があります。
RAM容量の制限から，RAMファイルには現在のところ2048バイトのスペースを２つあけられるのみですが，
プログラム容量によっては小型のものを多く作成することもできるでしょう。
ATL1200.cのコンパイル／実行前に，RAMディスクファイルのスペースを作っておいて下さい。
（RAMファイルのINITコマンド）

RAMファイルの中のATLコードをシリアルI/Fで入出力する場合には，
・ATL1200.cをテキストファイルエリアに待避する
・RAMディスクのプログラムをテキストエディットエリアに呼び出し，SIOで入出力する。
・ATL1200.cをテキストファイルエリアからエディットエリアに呼び出す
・テキストファイルエリアのATL1200コードを消去する
の手順をとって下さい。（めんどうじゃのぅ。）

■□ライタ回路／ポケコンPC-G850との接続□■
ATL/1200はPC-G850とAT90S1200の組み合わせでのみ動作を確認しています。
このポケコンは工業高校のみで扱っているらしく，
私はYahoo!オークションで入手しました。5000-10000円が相場のようです。
C言語を搭載して32Kb以上のメモリを持つ機種ならば移植は容易だと思います。
また，もしコードをBASICに移植できれば，PC-E200でも適用可能なはずです。
（私は「キッチュ食い」なのでそのようなことはできません。:-P）
また，８ピンのAT90S2323にもおそらく適用可能と思いますが，確認はしていません。

PC-G850へのCプログラムの送受信にはアダプタが必要になります。

KS-World（http://www.rupis.ne.nu/）中の記事:
http://www.rupis.ne.nu/pc-poke.htm

くりこうさんのページ（http://www1.plala.or.jp/kurikou/index.html）中の記事
http://www1.plala.or.jp/kurikou/pokekon/poke_f.html

の中の記事を参考にして製作なさってみて下さい。
私のアダプタはMAX232を用いたものですが，昔のマックで問題なく9600bpsで通信できています。
製作が面倒ですか？『打ち込む』ほうがもっともっともっともっと面倒のはずです。
（昔のパソコン雑誌を思い出します．．．。）


以下のの回路図を参考にライタを製作して，PC-G850向かって左側の11ピンI/Oポートに接続します。
PC-G850のピンは向かって上から1...11の順です。
AT90S1200のピンはTTLの配置と同じですが，詳しくはATMELのページからデータシートを見て下さい。

［AT90S1200］　　　　　　　　　　［PC-G850ポケコン］

20:Vcc -------------------------- 2:Vcc
                    ｜
                    ＝ 0.1μF
                    ｜
                   ----
                   /// GND

10:GND -------------------------- 3:GND

                       1kΩ
19:SCK <---------------VVV------< 4 
                ｜          ｜
                ＝1nF       ｜
                ｜     100k ｜
                 ------VVV---
                ｜
               ----
               /// GND

                       1kΩ
18:MISO>---------------VVV------> 5
                       100k ｜
                 ------VVV---
                ｜
               ----
               /// GND

                       1kΩ
17:MOSI<---------------VVV------< 6
                       100k ｜
                 ------VVV---
                ｜
               ----
               /// GND
                       1kΩ
1:*RESET<--------------VVV------< 7
                       100k ｜
                 ------VVV---
                ｜
               ----
               /// GND

                                  20p（XTALの場合）
4:XTAL1<--------------------------||----
          ｜  10MHz XTALまたは          ｜
           --｜口｜-- セラミック発振子  ｜
                    ｜            20p   ｜
5:XTAL2<--------------------------||----｜
        　　　　　　　　　　　　       ----
              　　　　　　　　　　　　 /// GND
　　　　　（セラミック発振子の場合は両端を4,5ピンに，まん中の端子をGNDに接続）


回路図中，SCKのパスコンの1nF(102の表示)と1kΩの組み合わせは守って下さい。
その他は多少数値が外れても大丈夫のはずです。
ISPの場合，10MHz以外の周波数では確認していません。
周波数によってはパスコンの値を変える必要があるかもしれません。

ポケコンから5Vを得ますので，外部電源は必要ありません。
今のところ書き込みは早いとはいいがたく，１００ワード書き込むのに１分以上かかりますがご容赦下さい。

■□将来的には□■
２バイト変数や数値演算の充実は，コンパイラのサイズが増大するので今のところ考えていません。
これらを行うならばむしろソースコードのライブラリを製作する方向が適当と思います。

また，#define，case文の使用数の限界以上になってしまっているので，これ以上命令も増やせないでしょう。
あくまでお手軽な簡易言語ですから．．．。

一部の変数とI/Oに対する定数代入／比較は残りのレジスタを使えばできるのですが，
そうすると条件分岐のプログラムが複雑になりますのでこれもペンディングです。
アセンブラがそこそこ文字数が少なくかけます，くらいに受け取って下さい。

また，もっと単一目的（機器制御，通信など）に特化したマクロな言語を作成するのも一つの方向と思います。
（こちらの方が面白くて脈がありそうです。次に作るとすればこれですね。）

バグや改善などのアドバイス歓迎です。ご遠慮なく。

■□免責事項□■
本プログラムによるいかなる不具合／誤動作／その他の予期せぬ結果について，
本プログラムの製作者は保証はいたしかねます。あくまで自己責任においてご使用下さい。

また，この説明書中の通信アダプタの動作についても同様であり，記事製作者への問い合わせはご遠慮下さい。

■□謝辞□■
この説明書中におきましては通信アダプタの回路図記事のリンクをご快諾頂きました。
この場をお借りしてかずやんさんとくりこうさんにお礼申し上げます。
```
