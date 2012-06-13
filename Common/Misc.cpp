//�w���v���b�Z�[�W�Ȃǂ������G���ȃN���X
#include "Misc.h"


CMisc::CMisc(void)
{
}

CMisc::~CMisc(void)
{
}

const wchar_t* CMisc::GetUsage(){

	wchar_t* buff =
L"�@-i infile [-nogui | -start [-quit]] \n"
L"�@�@�@�@�@�@[-o output [-dn]] [-srv servicenum]   \n"
L"�@�@�@�@�@�@[-es n] [-encode ffmpeg_parm] [-sound n] [-rf64] [-vf] [-delay n]\n"
L"�I�v�V����                           \n"
L"  -i infile   ���̓t�@�C��(ts)���w�肵�܂��B                   \n"
L"  -nogui      �E�C���h�E��\�������Ɏ����ŏ������J�n���܂��B   \n"
L"  -start      �E�C���h�E��\���������ŏ������J�n���܂��B       \n"
L"  -quit       �����I���㎩���ŏI�����܂��B                     \n"
L"  -o outfile  �o�̓t�@�C�����g���q���������`�Ŏw�肵�܂��B     \n"
L"              �i�g���q�͎����t�^����܂��B)                    \n"
L"  -dn         -o�Ŏw�肳�ꂽ�t�@�C�����ɂ��Ċg���q�̎����t�^���s���܂���B\n"
L"  -srv n      �����Ώۂ̃T�[�r�X�ԍ����w�肵�܂��B(10�i���l)   \n"
L"  -es n       ����ES���w�肵�܂��B                             \n"
L"                  0:�T�[�r�X�Ɉˑ�                             \n"
L"                  1-2:2-3�Ԗڂɑ��݂��鉹��(�Ȃ��ꍇ��0�ƂȂ�) \n"
L"  -encode     �G���R�[�h�������w�肵�܂��B                     \n"
L"                  ��FDemux(m2v+wav) �c����&�f��(�f�t�H���g)  \n"
L"                      Demux(wav)     �c�����̂�               \n"
L"                      Demux(m2v)     �c�f���̂�               \n"
L"                      MPEG2PS        �cMPG2PS�`��(cap_sts_sea.ini�Q��)\n"
L"                      WMV8           �cWMV8(cap_sts_sea.ini�Q��)      \n"
L"  -sound n    �������鉹����I�����܂��B                       \n"
L"                  0:Stereo(��+��)                              \n"
L"                  1:�剹��                                     \n"
L"                  2:������                                     \n"
L"                  3:����5.1ch                                  \n"
L"                  4:����5.1ch(ch����wav�t�@�C�����쐬)         \n"
L"  -rf64       WAV�t�@�C����4GB�𒴂������ɁAWAV RF64�`���ŏo�͂��܂��B\n"
L"              �i�w�肪�Ȃ��ꍇ��4GB�𒴂���ꍇ��RIFF�`���ł̏o�͂ƂȂ�܂�)\n"
L"  -vf         Video Frame�⊮��L���ɂ��܂�(29.97fps�O��)      \n"
L"  -nd         �X�N�����u���������s���܂���                     \n"
L"              �i�����ς݃t�@�C���̏ꍇ�A�����������ɂȂ�܂�)  \n"
L"  -delay n   �����x���␳���鎞�Ԃ�ݒ肵�܂��B(�P�� ms)       \n"
L"  -bg,-background                                              \n"
L"              �o�b�N�O���E���h���[�h�Ŏ��s���܂��B             \n"
L"              XP ... �v���Z�X�D��x���Ŏ��s���܂��B          \n"
L"              Vista�ȍ~ ... Low priority I/O���g�p���܂��B     \n"
L"  -help,-?,-h                                                  \n"
L"              ���̃��b�Z�[�W��\�����܂��B                     \n"
;

	return buff;
}

const wchar_t* CMisc::GetVersion()
{

	wchar_t* buff =L"BonTsDemux Version 1.10 mod.10k7 + nogui + es + fix05 \n";
	return buff;

}