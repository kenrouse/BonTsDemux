//ヘルプメッセージなどを扱う雑多なクラス
#include "Misc.h"


CMisc::CMisc(void)
{
}

CMisc::~CMisc(void)
{
}

const wchar_t* CMisc::GetUsage(){

	wchar_t* buff =
L"　-i infile [-nogui | -start [-quit]] \n"
L"　　　　　　[-o output [-dn]] [-srv servicenum]   \n"
L"　　　　　　[-es n] [-encode ffmpeg_parm] [-sound n] [-rf64] [-vf] [-delay n]\n"
L"オプション                           \n"
L"  -i infile   入力ファイル(ts)を指定します。                   \n"
L"  -nogui      ウインドウを表示せずに自動で処理を開始します。   \n"
L"  -start      ウインドウを表示し自動で処理を開始します。       \n"
L"  -quit       処理終了後自動で終了します。                     \n"
L"  -o outfile  出力ファイルを拡張子を除いた形で指定します。     \n"
L"              （拡張子は自動付与されます。)                    \n"
L"  -dn         -oで指定されたファイル名について拡張子の自動付与を行いません。\n"
L"  -srv n      処理対象のサービス番号を指定します。(10進数値)   \n"
L"  -es n       音声ESを指定します。                             \n"
L"                  0:サービスに依存                             \n"
L"                  1-2:2-3番目に存在する音声(ない場合は0となる) \n"
L"  -encode     エンコード方式を指定します。                     \n"
L"                  例：Demux(m2v+wav) …音声&映像(デフォルト)  \n"
L"                      Demux(wav)     …音声のみ               \n"
L"                      Demux(m2v)     …映像のみ               \n"
L"                      MPEG2PS        …MPG2PS形式(cap_sts_sea.ini参照)\n"
L"                      WMV8           …WMV8(cap_sts_sea.ini参照)      \n"
L"  -sound n    処理する音声を選択します。                       \n"
L"                  0:Stereo(主+副)                              \n"
L"                  1:主音声                                     \n"
L"                  2:副音声                                     \n"
L"                  3:強制5.1ch                                  \n"
L"                  4:強制5.1ch(ch毎にwavファイルを作成)         \n"
L"  -rf64       WAVファイルが4GBを超えた時に、WAV RF64形式で出力します。\n"
L"              （指定がない場合に4GBを超える場合はRIFF形式での出力となります)\n"
L"  -vf         Video Frame補完を有効にします(29.97fps前提)      \n"
L"  -nd         スクランブル解除を行いません                     \n"
L"              （解除済みファイルの場合、処理が高速になります)  \n"
L"  -delay n   音声遅延補正する時間を設定します。(単位 ms)       \n"
L"  -bg,-background                                              \n"
L"              バックグラウンドモードで実行します。             \n"
L"              XP ... プロセス優先度を低で実行します。          \n"
L"              Vista以降 ... Low priority I/Oを使用します。     \n"
L"  -help,-?,-h                                                  \n"
L"              このメッセージを表示します。                     \n"
;

	return buff;
}

const wchar_t* CMisc::GetVersion()
{

	wchar_t* buff =L"BonTsDemux Version 1.10 mod.10k7 + nogui + es + fix05 \n";
	return buff;

}