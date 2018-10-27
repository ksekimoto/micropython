/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Kentaro Sekimoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */

#include <stdbool.h>
#include "../esp8266/esp8266.h"

#include "common.h"
#include "ff.h"

extern FATFS *fatfs_sd;

#define WIFI_DATA_MAX    256

static unsigned char wifi_data[WIFI_DATA_MAX];
static int WiFiRecvOutlNum = -1; /* serial number from ESP8266 */
static int esp8266_ch = WIFI_SERIAL;
static int esp8266_baud = WIFI_BAUDRATE;

unsigned long millis() {
    return mtick();
}

static void esp8266_serial_begin(void) {
    sci_init(esp8266_ch, esp8266_baud);
}

static int esp8266_serial_available(void) {
    return sci_rx_any(esp8266_ch);
}

static int esp8266_serial_read(void) {
    return (int)sci_rx_ch(esp8266_ch);
}

static void esp8266_serial_write_byte(unsigned char c) {
    sci_tx_ch(esp8266_ch, (unsigned char)c);
}
static void esp8266_serial_write(unsigned char *s, int len) {
    for (int i = 0; i < len; i++) {
        sci_tx_ch(esp8266_ch, (unsigned char)s[i]);
    }
}

static void esp8266_serial_print(char *s) {
    sci_tx_str(esp8266_ch, s);
}

static void esp8266_serial_printi(int i) {
    char s[10];
    sprintf(s, "%d", i);
    sci_tx_str(esp8266_ch, s);
}

static void esp8266_serial_println(char *s) {
    sci_tx_str(esp8266_ch, s);
    sci_tx_ch(esp8266_ch, '\r');
    sci_tx_ch(esp8266_ch, '\n');
}

static void esp8266_serial_printiln(int i) {
    esp8266_serial_printi(i);
    sci_tx_ch(esp8266_ch, '\r');
    sci_tx_ch(esp8266_ch, '\n');
}

/* ToDo */
static void usb_serial_begin(void) {
}

static int usb_serial_available(void) {
    return (int)0;
}

static int usb_serial_read(void) {
    return (int)0;
}

static void usb_serial_write_byte(int c) {
}

static void usb_serial_write(unsigned char *s, int len) {
}

static void usb_serial_print(char *s) {
}

static void usb_serial_println(char *s) {
}

inline char *esp8266_data_ptr(void) {
    return (char *)&wifi_data[0];
}

inline int esp8266_data_len(void) {
    return (int)strlen(&wifi_data[0]);
}

//**************************************************
// OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、
// 指定されたシリアルポートに出力します
//
// 1:受信した, 0:受信できなかった 2:受信がオーバーフローした
//**************************************************
static int get_data(unsigned int wait_msec) {
    unsigned long times;
    int c;
    int okt = 0;
    int ert = 0;
    int len = 0;
    int n = 0;

    memset((void *)wifi_data, 0, sizeof(wifi_data));
    wifi_data[0] = 0;
    times = millis();
    while (n < 256) {
        if (millis() - times > wait_msec) {
            DBG_PRINT1("timeout");
            wifi_data[n] = 0;
            return 0;
        }
        while ((len = esp8266_serial_available()) != 0) {
            //DBG_PRINT1("len=",len);
            //DBG_PRINT1("n=",n);
            for (int i = 0; i < len; i++) {
                c = esp8266_serial_read();
                //指定のシリアルポートに出す設定であれば、受信値を出力します
                if (WiFiRecvOutlNum >= 0) {
                    esp8266_serial_write_byte((unsigned char)c);
                }
                //DBG_PRINT2("c=",c);
                wifi_data[n] = c;
                n++;
                if (c == 'O') {
                    okt++;
                    ert++;
                } else if (c == 'K') {
                    okt++;
                } else if (c == 0x0d) {
                    ert++;
                    okt++;
                } else if (c == 0x0a) {
                    ert++;
                    okt++;
                    if (okt == 4 || ert == 7) {
                        // OK 0d0a || ERROR 0d0a
                        wifi_data[n] = 0;
                        DBG_PRINT1((const char *)wifi_data);
                        return 1;
                        //n = 256;
                    } else {
                        ert = 0;
                        okt = 0;
                    }
                } else if (c == 'E' || c == 'R') {
                    ert++;
                } else {
                    okt = 0;
                    ert = 0;
                }
            }
            times = millis();
        }
    }
    return 2;
}

//**************************************************
// ステーションモードの設定: WiFi.cwmode
//  mode: 1:Station, 2:SoftAP, 3:Station + SoftAP
//**************************************************
void esp8266_cwmode(int mode) {
    esp8266_serial_print("AT+CWMODE=");
    esp8266_serial_printiln(mode);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// コマンド応答のシリアル出力設定: WiFi.serialOut
//  WiFi.serialOut( mode[,serialNumber] )
//  mode: 0:出力しない, 1:出力する
//  serialNumber: 出力先のシリアル番号
//**************************************************
void esp8266_serialout(int mode, int num) {
    int n;
    if (mode == 0) {
        WiFiRecvOutlNum = -1;
    } else {
        if (n >= 2) {
            if (num >= 0) {
                WiFiRecvOutlNum = num;
            }
        }
    }
}

//**************************************************
// ATコマンドの送信: WiFi.at
//  WiFi.at( command[, mode] )
//  commnad: ATコマンド文字列
//  mode: 0:'AT+'を自動追加する、1:'AT+'を自動追加しない
//**************************************************
void esp8266_at(int n, int mode, char *s) {
    int len = strlen(s);
    if (n <= 1 || mode == 0) {
        esp8266_serial_print("AT+");
    }
    for (int i = 0; i < 254; i++) {
        if (i >= len) {
            break;
        }
        wifi_data[i] = s[i];
    }
    wifi_data[len] = 0;

    esp8266_serial_println((const char*)wifi_data);
    //DBG_PRINT2("WiFi.at",(const char*)wifi_data);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// WiFi接続します: WiFi.connect
//  WiFi.connect(SSID,Passwd)
//  WiFi.cwjap(SSID,Passwd)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//**************************************************
void esp8266_cwjap(char *ssid, char *pass) {
    int slen = strlen(ssid);
    int plen = strlen(pass);

    esp8266_serial_print("AT+CWJAP=");
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0;
    esp8266_serial_print((const char*)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= slen) {
            break;
        }
        wifi_data[i] = ssid[i];
    }
    wifi_data[slen] = 0;
    esp8266_serial_print((const char*)wifi_data);
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0x2C;     //-> ,
    wifi_data[2] = 0x22;     //-> "
    wifi_data[3] = 0;
    esp8266_serial_print((const char*)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= plen) {
            break;
        }
        wifi_data[i] = pass[i];
    }
    wifi_data[plen] = 0;
    esp8266_serial_print((const char*)wifi_data);
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0;
    esp8266_serial_println((const char*)wifi_data);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// WiFiアクセスポイントになります: WiFi.softAP
//  WiFi.softAP(SSID,Passwd,Channel,Encrypt)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//  Channel: チャネル
//  Encrypt: 暗号タイプ 0:Open, 1:WEP, 2:WPA_PSK, 3:WPA2_PSK, 4:WPA_WPA2_PSK
//**************************************************
void esp8266_softap(char *ssid, char *pass, int ch, int enc) {
    int slen = strlen(ssid);
    int plen = strlen(pass);

    if (enc < 0 || enc > 4) {
        enc = 0;
    }
    esp8266_serial_print("AT+CWSAP=");
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0;
    esp8266_serial_print((const char*)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= slen) {
            break;
        }
        wifi_data[i] = ssid[i];
    }
    wifi_data[slen] = 0;
    esp8266_serial_print((const char*)wifi_data);
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0x2C;     //-> ,
    wifi_data[2] = 0x22;     //-> "
    wifi_data[3] = 0;
    esp8266_serial_print((const char*)wifi_data);
    for (int i = 0; i < 254; i++) {
        if (i >= plen) {
            break;
        }
        wifi_data[i] = pass[i];
    }
    wifi_data[plen] = 0;
    esp8266_serial_print((const char*)wifi_data);
    wifi_data[0] = 0x22;     //-> "
    wifi_data[1] = 0x2C;     //-> ,
    wifi_data[2] = 0;
    esp8266_serial_print((const char*)wifi_data);
    esp8266_serial_print(ch);
    esp8266_serial_print(",");
    esp8266_serial_println(enc);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// アクセスポイントに接続されているIP取得: WiFi.connetedIP
//  WiFi.connectedIP()
//**************************************************
void esp8266_connectedip(void) {
    esp8266_serial_println("AT+CWLIF");
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// DHCP有無の切り替え: WiFi.dhcp
//  WiFi.dhcp(mode, bool)
//  mode: 0:SoftAP, 1:Station, 2:Both softAP + Station
//  bool: 0:disable , 1:enable
//**************************************************
void esp8266_dhcp(int mode, int bl) {
    esp8266_serial_print("AT+CWDHCP=");
    esp8266_serial_print(mode);
    esp8266_serial_print(",");
    esp8266_serial_println(bl);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// IPアドレスとMACアドレスの表示: WiFi.ipconfig
//  WiFi.ipconfig()
//  WiFi.cifsr()
//**************************************************
void esp8266_cifsr(void) {
    esp8266_serial_println("AT+CIFSR");
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

//**************************************************
// USBポートとESP8266をシリアルで直結します: WiFi.bypass
//  WiFi.bypass()
// リセットするまで、処理は戻りません。
//**************************************************
void esp8266_bypass(void) {
    int len0, len1, len;
    while (true) {
        len0 = usb_serial_available();
        len1 = esp8266_serial_available();
        if (len0 > 0) {
            len = len0 < 256 ? len0 : 256;
            for (int i = 0; i < len; i++) {
                wifi_data[i] = (unsigned char)usb_serial_read();
            }
            esp8266_serial_write(wifi_data, len);
        }
        if (len1 > 0) {
            len = len1 < 256 ? len1 : 256;
            for (int i = 0; i < len; i++) {
                wifi_data[i] = (unsigned char)esp8266_serial_read();
            }
            usb_serial_write(wifi_data, len);
        }
    }
}

void esp8266_version(void) {
    esp8266_serial_println("AT+GMR");
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

void esp8266_disconnect(void) {
    esp8266_serial_println("AT+CWQAP");
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

void esp8266_multiconnect(int mode) {
    esp8266_serial_print("AT+CIPMUX=");
    esp8266_serial_printiln(mode);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
}

bool sd_exists(char *fn) {
    FRESULT res;
    FILINFO fno;
    res = f_stat(fatfs_sd, fn, &fno);
    return (res == FR_OK);
}

bool sd_remove(char *fn) {
    FRESULT res;
    res = f_unlink(fatfs_sd, fn);
    return (res == FR_OK);
}

bool sd_open(FIL *fp, char *fn, unsigned char mode) {
    FRESULT res;
    res = f_open(fatfs_sd, fp, fn, mode);
    return (res == FR_OK);
}

int sd_read_byte(FIL *fp) {
    unsigned char c;
    int len = 0;
    FRESULT res = f_read(fp, &c, 1, &len);
    if (len == 0) {
        return -1;
    }
    return (int)c;
}

int sd_read(FIL *fp, unsigned char *buf, int size) {
    int len;
    FRESULT res = f_read(fp, buf, size, &len);
    return len;
}

int sd_write_byte(FIL *fp, unsigned char c) {
    int len;
    FRESULT res = f_write(fp, &c, 1, &len);
    return len;
}

int sd_write(FIL *fp, unsigned char *buf, int size) {
    int len;
    FRESULT res = f_write(fp, buf, size, &len);
    return len;
}

void sd_seek(FIL *fp, unsigned long pos) {
    FSIZE_t ofs = (FSIZE_t)pos;
    FRESULT res = f_lseek(fp, ofs);
}

unsigned long sd_size(FIL *fp) {
    return (unsigned long)f_size(fp);
}

void sd_flush(FIL *fp) {
    f_sync(fp);
}

void sd_close(FIL *fp) {
    f_close(fp);
}

//**************************************************
// ファイルに含まれる+IPDデータを削除します
// ipd: ipdデータ列
// strFname1: 元ファイル
// strFname2: 削除したファイル
//**************************************************
int cut_garbage_data(const char *ipd, const char *strFname1,
        const char *strFname2) {
    FIL fp, fd;

    if (sd_exists(strFname2)) {
        sd_remove(strFname2);
    }
    if (!sd_open(&fp, strFname1, FA_READ)) {
        return 2;
    }
    if (!sd_open(&fd, strFname2, FA_WRITE | FA_CREATE_NEW)) {
        return 3;
    }
    int ipdLen = strlen(ipd);
    int cnt;
    unsigned char c;
    int rc;
    bool findFlg = true;
    unsigned char str[16];
    int dLen;
    unsigned long seekCnt = 0;
    while (true) {
        //+IPD文字列を探します
        cnt = 0;
        while (true) {
            rc = sd_read_byte(&fp);
            if (rc < 0) {
                findFlg = false;
                break;
            }
            c = (unsigned char)rc;
            if (ipd[cnt] == c) {
                cnt++;
                if (cnt == ipdLen) {
                    seekCnt += cnt;
                    break;
                }
            } else if (c == 0x0D) {
                cnt = 1;
            } else {
                cnt = 0;
            }
        }
        //Serial.print("findFlg= ");
        //Serial.println(findFlg);
        if (findFlg == false) {
            break;
        }
        //ここから後はバイト数が来ているはず
        cnt = 0;
        while (true) {
            rc = sd_read_byte(&fp);
            if (rc < 0) {
                findFlg = false;
                break;
            }
            c = (unsigned char)rc;
            str[cnt] = c;
            if (c == ':') {
                str[cnt] = 0;
                seekCnt += cnt + 1;
                break;
            } else if (cnt >= 15) {
                str[15] = 0;
                findFlg = false;
                break;
            }
            cnt++;
        }
        if (findFlg == false) {
            break;
        }
        //読み込むバイト数を求めます
        dLen = atoi((const char*)str);
        seekCnt += dLen;
        //Serial.print("dLen= ");
        //Serial.println((const char*)str);
        while (dLen > 0) {
            if (dLen >= 256) {
                sd_read(&fp, wifi_data, 256);
                sd_write(&fd, wifi_data, 256);
                dLen -= 256;
            } else {
                sd_read(&fp, wifi_data, dLen);
                sd_write(&fd, wifi_data, dLen);
                dLen = 0;
            }
        }
    }
    if (findFlg == false) {
        //処理していないところは、そのまま書きます
        sd_seek(&fp, seekCnt);

        while (true) {
            dLen = sd_read(&fp, wifi_data, 256);
            sd_write(&fd, wifi_data, dLen);
            if (dLen < 256) {
                break;
            }
        }
    }
    sd_flush(&fd);
    sd_close(&fd);
    sd_close(&fp);
    return 1;
}

//**************************************************
// http GETをSDカードに保存します: WiFi.httpGetSD
//  WiFi.httpGetSD( Filename, URL[,Headers] )
//  Filename: 保存するファイル名
//  URL: URL
//  Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//      0: 失敗
//      1: 成功
//      2: SDカードが使えない
//      ... 各種エラー
//**************************************************
int esp8266_get_sd(char *strURL, char *strFname, int n, char **head, int ssl) {
    const char *tmpFilename = "wifitmp.tmp";
    const char *hedFilename = "hedrfile.tmp";
    int len = 0;
    FIL fp, fd;
    int sla, koron;
    //httpサーバに送信するデータを、strFname ファイルに生成します。
    //既にファイルがあれば消す
    if (sd_exists(hedFilename)) {
        sd_remove(hedFilename);
    }
    //ファイルオープン
    if (!sd_open(&fp, hedFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 3;
    }
    //1行目を生成
    {
        sd_write(&fp, (unsigned char*)"GET /", 5);
        //httpsかチェック
        if (ssl) {
            DBG_PRINT1("AT+CIPSSLSIZE=4096");
            esp8266_serial_println("AT+CIPSSLSIZE=4096");
            get_data(WIFI_WAIT_MSEC);
        }
        //URLからドメインを分割する
        len = strlen(strURL);
        sla = len;
        koron = 0;
        for (int i = 0; i < len; i++) {
            if (strURL[i] == '/') {
                sla = i;
                break;
            }
            if (strURL[i] == ':') {
                koron = i;
            }
        }
        for (int i = sla + 1; i < len; i++) {
            sd_write(&fp, &strURL[i], 1);
        }
        sd_write(&fp, (unsigned char*)" HTTP/1.1", 9);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //Hostヘッダを生成
    {
        sd_write(&fp, (unsigned char *)"Host: ", 6);
        if (koron == 0) {
            koron = sla;
        }
        for (int i = 0; i < koron; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //ヘッダ情報が追加されているとき
    for (int i = 0; i < n; i++) {
        len = strlen(*head);
        //ヘッダの追記
        sd_write(&fp, (const unsigned char*)*head++, len);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //改行のみの行を追加する
    sd_write_byte(&fp, 0x0D);
    sd_write_byte(&fp, 0x0A);
    sd_flush(&fp);
    sd_close(&fp);
    //****** AT+CIPSTARTコマンド ******
    //wifi_data[]に、ドメインとポート番号を取得
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_serial_print("AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_serial_print("AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_serial_print((const char*)wifi_data);
    esp8266_serial_print("\",");
    if (koron < sla) {
        esp8266_serial_println((const char*)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_serial_println("443");
        } else {
            esp8266_serial_println("80");
        }
    }
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //Serial.print("httpServer Connect: ");
    //Serial.print((const char*)wifi_data);
    //****** AT+CIPSEND コマンド ******
    //送信データサイズ取得
    if (!sd_open(&fp, hedFilename, FA_READ)) {
        return 4;
    }
    //ファイルサイズ取得
    int sByte = (int)sd_size(&fp);
    sd_close(&fp);
    //Serial.print("AT+CIPSEND=4,");
    esp8266_serial_print("AT+CIPSEND=4,");
    sprintf((char*)wifi_data, "%d", sByte);
    //Serial.println((const char*)wifi_data);
    esp8266_serial_println((const char*)wifi_data);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //Serial.print("> Waiting: ");
    //Serial.print((const char*)wifi_data);
    //****** 送信データ受付モードになったので、http GETデータを送信する ******
    {
        if (!sd_open(&fp, hedFilename, FA_READ)) {
            return 5;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sByte; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            //Serial.print((const char*)wifi_data);
            esp8266_serial_print((const char*)wifi_data);
        }
        sd_close(&fp);
        sd_remove(tmpFilename); //受信するためファイルを事前に消している
        //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
                || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
            DBG_PRINT1("WIFI ERR");
            return 0;
        }
        //Serial.print("Send Finish: ");
        //Serial.print((const char*)wifi_data);
    }
    //****** 送信終了 ******
    //****** 受信開始 ******
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    unsigned char recv[2];
    times = millis();
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                recv[0] = (unsigned char)esp8266_serial_read();
                sd_write(&fp, (unsigned char*)recv, 1);
            }
            times = millis();
            wait_msec = 1000;   //データが届き始めたら、1sec待ちに変更する
        }
    }
    sd_flush(&fp);
    sd_close(&fp);
    //****** 受信終了 ******
    //Serial.println("Recv Finish");
    //受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
    int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename, strFname);
    if (ret != 1) {
        return 7;
    }
    //****** AT+CIPCLOSE コマンド ******
    DBG_PRINT1("AT+CIPCLOSE=4");
    esp8266_serial_println("AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    //Serial.println((const char*)wifi_data);
    return 1;
}

//**************************************************
// http GETプロトコルを送信する: WiFi.httpGet
//  WiFi.httpGet( URL[,Headers] )
//　送信のみで、結果を受信しない
//  URL: URL
//  Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//      0: 失敗
//      1: 成功
//**************************************************
int esp8266_get(unsigned char *strURL, unsigned char **hes, int n, int ssl) {
    int len = 0;
    int sla, cnt;
    int koron = 0;
    char sData[1024];

    //httpsかチェック
    if (ssl) {
        DBG_PRINT1("AT+CIPSSLSIZE=4096");
        esp8266_serial_println("AT+CIPSSLSIZE=4096");
        get_data(WIFI_WAIT_MSEC);
    }
    //URLからドメインを分割する
    len = strlen(strURL);
    sla = len;
    for (int i = 0; i < len; i++) {
        if (strURL[i] == '/') {
            sla = i;
            break;
        }
        if (strURL[i] == ':') {
            koron = i;
        }
    }
    if (koron == 0) {
        koron = sla;
    }
    //****** AT+CIPSTARTコマンド ******
    //wifi_data[]に、ドメインとポート番号を取得
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_serial_print("AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_serial_print("AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_serial_print((const char*)wifi_data);
    esp8266_serial_print("\",");
    if (koron < sla) {
        esp8266_serial_println((const char*)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_serial_println("443");
        } else {
            esp8266_serial_println("80");
        }
    }
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //****** AT+CIPSEND コマンド ******
    //ヘッダの1行目を生成
    {
        strcpy(sData, "GET /");
        cnt = 5;
        for (int i = sla + 1; i < len; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, " HTTP/1.1\r\n");
    }
    //Hostヘッダを生成
    {
        strcat(sData, "Host: ");
        cnt = strlen(sData);
        for (int i = 0; i < koron; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, "\r\n");
    }
    //ヘッダ情報が追加されているとき
    for (int i = 0; i < n; i++) {
            len = strlen(*hes);
            //ヘッダの追記
            strcat(sData, *hes++);
            strcat(sData, "\r\n");
    }
    //改行のみの行を追加する
    strcat(sData, "\r\n");
    //送信データサイズ取得
    len = strlen(sData);
    DBG_PRINT1((const char *)sData);
    esp8266_serial_print("AT+CIPSEND=4,");
    esp8266_serial_printiln(len);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //****** 送信データ受付モードになったので、http GETデータを送信する ******
    {
        esp8266_serial_print((const char*)sData);
        //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
                || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
            DBG_PRINT1("WIFI ERR");
            return 0;
        }
    }
    //****** 送信終了 ******
    //****** 受信開始 ******
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    times = millis();
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                esp8266_serial_read();
            }
            times = millis();
            wait_msec = 100;    //データが届き始めたら、100ms待ちに変更する
        }
    }
    //****** 受信終了 ******
    //****** AT+CIPCLOSE コマンド ******
    esp8266_serial_println("AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}

//**************************************************
// TCP/UDP接続を閉じる: WiFi.cClose
//  WiFi.cClose(number)
//  number: 接続番号(1～4)
//**************************************************
void esp8266_cclose(int num) {
    esp8266_serial_print("AT+CIPCLOSE=");
    esp8266_serial_printiln(num);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    return;
}

//**************************************************
// UDP接続を開始します: WiFi.udpOpen
//  WiFi.udpOpen( number, IP_Address, SendPort, ReceivePort )
//　number: 接続番号(1～4)
//  IP_Address: 通信相手アドレス
//  SendPort: 送信ポート番号
//  ReceivePort: 受信ポート番号
//**************************************************
static int chk_OK() {
    char *p = (char *)wifi_data;
    int n = strlen((const char *)wifi_data);
    DBG_PRINT1((const char *)wifi_data);
    if (n >= 4) {
        if ((p[n - 4] == 'O') && (p[n - 3] == 'K'))
            return 1;
    }
    return 0;
}

int esp8266_udpopen(int num, char *strIpAdd, int sport, int rport) {
    //****** AT+CIPSTARTコマンド ******
    esp8266_serial_print("AT+CIPSTART=");
    esp8266_serial_printi(num);
    esp8266_serial_print(",\"UDP\",\"");
    esp8266_serial_print((const char*)strIpAdd);
    esp8266_serial_print("\",");
    esp8266_serial_printi(sport);
    esp8266_serial_print(",");
    esp8266_serial_printiln(rport);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    return chk_OK();
}

//**************************************************
// 指定接続番号にデータを送信します: WiFi.send
//  WiFi.send( number, Data[, length] )
//　number: 接続番号(0～3)
//  Data: 送信するデータ
//　length: 送信データサイズ
//
//  戻り値は
//    送信データサイズ
//**************************************************
int esp8266_send(int num, char *strdata, int len) {
    //****** AT+CIPSTARTコマンド ******
    esp8266_serial_print("AT+CIPSEND=");
    esp8266_serial_print(num);
    esp8266_serial_print(",");
    esp8266_serial_println(len);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        return 0;
    }
    //esp8266_serial_print((const char*)strdata);
    for (int i = 0; i < len; i++) {
        esp8266_serial_print((char)strdata[i]);
    }
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        //タイムアウトと思われるので、強制的にデータサイズの不足分の0x0Dを送信する
        for (int i = 0; i < len - (int)strlen(strdata); i++) {
            esp8266_serial_print("\r");
        }
        return 0;
    }
    return len;
}

//**************************************************
// 指定接続番号からデータを受信します: WiFi.recv
//  WiFi.recv( number )
//　number: 接続番号(0～3)
//
//  戻り値は
//    受信したデータの配列　ただし、256以下
//**************************************************
static char *esp8266_recv_buf[256];
int esp8266_recv(int num, char *recv_buf, int *recv_cnt) {
    unsigned char str[16];
    sprintf((char*)str, "\r\n+IPD,%d,", num);
    //if(esp8266_serial_available() == 0){
    //  return -1;
    //}
    //****** 受信開始 ******
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    times = millis();
    int len = strlen((char*)str);
    int cnt = 0;
    unsigned char c;
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        if (esp8266_serial_available()) {
            c = (unsigned char)esp8266_serial_read();
            if (str[cnt] == c) {
                cnt++;
                if (cnt == len) {
                    break;
                }
            } else if (c == 0x0D) {
                cnt = 1;
            } else {
                cnt = 0;
            }
            times = millis();
            wait_msec = 100;    //データが届き始めたら、100ms待ちに変更する
        }
    }
    //ここから後はバイト数が来ているはず
    times = millis();
    cnt = 0;
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            str[cnt] = 0;
            break;
        }
        if (esp8266_serial_available()) {
            c = (unsigned char)esp8266_serial_read();
            str[cnt] = c;
            if (c == ':') {
                str[cnt] = 0;
                break;
            } else if (cnt >= 15) {
                str[15] = 0;
                break;
            }
            cnt++;
            times = millis();
        }
    }
    len = atoi((const char*)str);
    //Serial.print("len= ");
    //Serial.println(len);
    //データを取りだします
    times = millis();
    *recv_cnt = 0;
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        if (esp8266_serial_available()) {
            recv_buf[*recv_cnt] = (char)esp8266_serial_read();
            (*recv_cnt)++;
            if (*recv_cnt >= len) {
                break;
            }
            times = millis();
        }
    }
    //****** 受信終了 ******
    return 1;
}

//**************************************************
// http POSTとしてSDカードのファイルをPOSTします: WiFi.httpPostSD
//  WiFi.httpPostSD( URL, Headers, Filename, Filename )
//  URL: URL
//  Headers: ヘッダに追記する文字列の配列
//  Filename: POSTするファイル名
//
//  戻り値は以下のとおり
//      0: 失敗
//      1: 成功
//      2: SDカードが使えない
//      ... 各種エラー
//**************************************************
int esp8266_post_sd(char *strURL, char *strSFname, char *strDFname, int n,
        char **head, int ssl) {
    const char *tmpFilename = "wifitmp.tmp";
    const char *headFilename = "headfile.tmp";
    int len = 0;
    FIL fp, fd;
    int sla, koron;
    int sBody, sHeader;
    //送信ファイルサイズ取得
    if (!sd_open(&fp, strSFname, FA_READ)) {
        return 3;
    }
    //ファイルサイズ取得
    sBody = (int)sd_size(&fp);
    sd_close(&fp);
    //httpサーバに送信するデータを、strFname ファイルに生成します。
    //既にファイルがあれば消す
    if (sd_exists(headFilename)) {
        sd_remove(headFilename);
    }
    //ファイルオープン
    if (!sd_open(&fp, headFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 4;
    }
    esp8266_serial_println("AT+CIPMUX=1");
    get_data(WIFI_WAIT_MSEC);
    //1行目を生成
    {
        sd_write(&fp, (unsigned char*)"POST /", 6);
        //httpsかチェック
        if (ssl) {
            DBG_PRINT1("AT+CIPSSLSIZE=4096");
            esp8266_serial_println("AT+CIPSSLSIZE=4096");
            get_data(WIFI_WAIT_MSEC);
        }
        //URLからドメインを分割する
        len = strlen(strURL);
        sla = len;
        koron = 0;
        for (int i = 0; i < len; i++) {
            if (strURL[i] == '/') {
                sla = i;
                break;
            }
            if (strURL[i] == ':') {
                koron = i;
            }
        }
        for (int i = sla + 1; i < len; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write(&fp, (unsigned char*)" HTTP/1.1", 9);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //Hostヘッダを生成
    {
        sd_write(&fp, (unsigned char*)"Host: ", 6);
        if (koron == 0) {
            koron = sla;
        }
        for (int i = 0; i < koron; i++) {
            sd_write_byte(&fp, strURL[i]);
        }
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //Content-Lengthを付けます
    {
        sd_write(&fp, (unsigned char*)"Content-Length: ", 16);
        sprintf((char*)wifi_data, "%u", sBody);
        sd_write(&fp, wifi_data, strlen((char*)wifi_data));
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //ヘッダ情報が追加されているとき
    for (int i = 0; i < n; i++) {
        len = strlen(head[i]);
        //ヘッダの追記
        sd_write(&fp, head[i], len);
        sd_write_byte(&fp, 0x0D);
        sd_write_byte(&fp, 0x0A);
    }
    //改行のみの行を追加する
    sd_write_byte(&fp, 0x0D);
    sd_write_byte(&fp, 0x0A);
    sd_flush(&fp);
    sd_close(&fp);
    //****** AT+CIPSTARTコマンド ******
    //wifi_data[]に、ドメインとポート番号を取得
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_serial_print("AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_serial_print("AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_serial_print((const char*)wifi_data);
    esp8266_serial_print("\",");
    if (koron < sla) {
        esp8266_serial_println((const char*)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_serial_println("443");
        } else {
            esp8266_serial_println("80");
        }
    }
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //Serial.print("httpServer Connect: ");
    //Serial.print((const char*)wifi_data);
    //****** AT+CIPSEND コマンド ******
    //送信ヘッダのサイズ取得
    if (!sd_open(&fp, headFilename, FA_READ)) {
        return 5;
    }
    //ファイルサイズ取得
    sHeader = (int)sd_size(&fp);
    sd_close(&fp);
    //Serial.print("AT+CIPSEND=4,");
    esp8266_serial_print("AT+CIPSEND=4,");
    sprintf((char*)wifi_data, "%u", sHeader + sBody);
    //Serial.println((const char*)wifi_data);
    esp8266_serial_println((const char*)wifi_data);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //Serial.print("> Waiting: ");
    //Serial.print((const char*)wifi_data);
    //****** 送信データ受付モードになったので、http POSTデータを送信する ******
    {
        //先ずヘッダデータを送信する
        if (!sd_open(&fp, headFilename, FA_READ)) {
            return 6;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sHeader; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            //Serial.print((const char*)wifi_data);
            esp8266_serial_print((const char*)wifi_data);
        }
        sd_close(&fp);
        //先ずボディデータを送信する
        if (!sd_open(&fp, strSFname, FA_READ)) {
            return 7;
        }
        wifi_data[1] = 0;
        for (int i = 0; i < sBody; i++) {
            wifi_data[0] = (unsigned char)sd_read_byte(&fp);
            //Serial.print((const char*)wifi_data);
            esp8266_serial_print((const char*)wifi_data);
        }
        sd_close(&fp);
        //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
                || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
            DBG_PRINT1("WIFI ERR");
            return 0;
        }
        //Serial.print("Send Finish: ");
        //Serial.print((const char*)wifi_data);
    }
    //****** 送信終了 ******
    //****** 受信開始 ******
    if (sd_exists(tmpFilename)) {
        sd_remove(tmpFilename);
    }
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    unsigned char recv[2];
    times = millis();
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                // esp8266_serial_read();
                recv[0] = (unsigned char)esp8266_serial_read();
                if (n >= 4) {
                    sd_write(&fp, (unsigned char*)recv, 1);
                }
            }
            times = millis();
            wait_msec = 100;    //データが届き始めたら、100ms待ちに変更する
        }
    }
    sd_flush(&fp);
    sd_close(&fp);
    //****** 受信終了 ******
    //Serial.println("Recv Finish");
    if (strDFname) {
        //受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
        int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename,
                (const char *)strDFname);
        if (ret != 1) {
            return 7;
        }
    }
    //****** AT+CIPCLOSE コマンド ******
    esp8266_serial_println("AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}

//**************************************************
// http POSTする: WiFi.httpPost
//  WiFi.httpPost( URL, Headers, data )
//　送信のみで、結果を受信しない
//  URL: URL
//  Headers: ヘッダに追記する文字列の配列
//　Data: POSTデータ
//
//  戻り値は以下のとおり
//      0: 失敗
//      1: 成功
//**************************************************
int esp8266_post(char *strURL, char *strData, char *strDFname, int n, char **head,
        int ssl) {
    const char *tmpFilename = "wifitmp.tmp";
    int sBody, sHeader;
    int sla, cnt;
    int koron = 0;
    char sData[1024];
    int len;
    FIL fp, fd;
    sBody = strlen(strData);
    esp8266_serial_println("AT+CIPMUX=1");
    get_data(WIFI_WAIT_MSEC);
    //httpsかチェック
    if (ssl) {
        DBG_PRINT1("AT+CIPSSLSIZE=4096");
        esp8266_serial_println("AT+CIPSSLSIZE=4096");
        get_data(WIFI_WAIT_MSEC);
    }
    //URLからドメインを分割する
    len = strlen(strURL);
    sla = len;
    for (int i = 0; i < len; i++) {
        if (strURL[i] == '/') {
            sla = i;
            break;
        }
        if (strURL[i] == ':') {
            koron = i;
        }
    }
    if (koron == 0) {
        koron = sla;
    }
    //****** AT+CIPSTARTコマンド ******
    //wifi_data[]に、ドメインとポート番号を取得
    for (int i = 0; i < sla; i++) {
        wifi_data[i] = strURL[i];
        if (i == koron) {
            wifi_data[i] = 0;
        }
    }
    wifi_data[sla] = 0;
    if (ssl) {
        esp8266_serial_print("AT+CIPSTART=4,\"SSL\",\"");
    } else {
        esp8266_serial_print("AT+CIPSTART=4,\"TCP\",\"");
    }
    esp8266_serial_print((const char*)wifi_data);
    esp8266_serial_print("\",");
    if (koron < sla) {
        esp8266_serial_println((const char*)&wifi_data[koron + 1]);
    } else {
        if (ssl) {
            esp8266_serial_println("443");
        } else {
            esp8266_serial_println("80");
        }
    }
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //****** AT+CIPSEND コマンド ******
    //ヘッダの1行目を生成
    {
        strcpy(sData, "POST /");
        cnt = 6;
        for (int i = sla + 1; i < len; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, " HTTP/1.1\r\n");
    }
    //Hostヘッダを生成
    {
        strcat(sData, "Host: ");
        cnt = strlen(sData);
        for (int i = 0; i < koron; i++) {
            sData[cnt] = strURL[i];
            cnt++;
        }
        sData[cnt] = 0;
        strcat(sData, "\r\n");
    }
    //Content-Lengthを付けます
    {
        strcat(sData, "Content-Length: ");
        sprintf((char*)wifi_data, "%d", sBody);
        strcat(sData, (char*)wifi_data);
        strcat(sData, "\r\n");
    }
    //ヘッダ情報を付けます
    for (int i = 0; i < n; i++) {
        //ヘッダの追記
        strcat(sData, head[i]);
        strcat(sData, "\r\n");
    }
    //改行のみの行を追加する
    strcat(sData, "\r\n");
    //送信データサイズ取得
    sHeader = strlen(sData);
    len = sHeader + sBody;
    esp8266_serial_print("AT+CIPSEND=4,");
    esp8266_serial_printiln(len);
    //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
    get_data(WIFI_WAIT_MSEC);
    if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
            || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
        DBG_PRINT1("WIFI ERR");
        return 0;
    }
    //****** 送信データ受付モードになったので、http POSTデータを送信する ******
    {
        //ヘッダを送信する
        esp8266_serial_print((const char*)sData);
        DBG_PRINT1(sData);
        //ボディを送信する
        esp8266_serial_print((const char*)strData);
        DBG_PRINT1(strData);
        //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読むか、指定されたシリアルポートに出力します
        get_data(WIFI_WAIT_MSEC);
        if (!(wifi_data[strlen((const char*)wifi_data) - 2] == 'K'
                || wifi_data[strlen((const char*)wifi_data) - 3] == 'K')) {
            DBG_PRINT1("WIFI ERR");
            return 0;
        }
    }
    //****** 送信終了 ******
    //****** 受信開始 ******
    if (sd_exists(tmpFilename)) {
        sd_remove(tmpFilename);
    }
    if (!sd_open(&fp, tmpFilename, FA_WRITE | FA_CREATE_NEW)) {
        return 6;
    }
    unsigned long times;
    unsigned int wait_msec = WIFI_WAIT_MSEC;
    unsigned char recv[2];
    times = millis();
    while (true) {
        //wait_msec 待つ
        if (millis() - times > wait_msec) {
            break;
        }
        while ((len = esp8266_serial_available()) != 0) {
            for (int i = 0; i < len; i++) {
                //esp8266_serial_read();
                recv[0] = (unsigned char)esp8266_serial_read();
                sd_write(&fp, (unsigned char*)recv, 1);
            }
            times = millis();
            wait_msec = 100;    //データが届き始めたら、100ms待ちに変更する
        }
    }
    sd_flush(&fp);
    sd_close(&fp);
    //****** 受信終了 ******
    if (strDFname) {
        //受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
        int ret = cut_garbage_data("\r\n+IPD,4,", tmpFilename,
                (const char *)strDFname);
        if (ret != 1) {
            return 7;
        }
    }
    //****** AT+CIPCLOSE コマンド ******
    esp8266_serial_println("AT+CIPCLOSE=4");
    get_data(WIFI_WAIT_MSEC);
    return 1;
}

int esp8266_init(void) {
    //ESP8266からの受信を出力しないに設定
    WiFiRecvOutlNum = -1;
    //CTS用にPIN15をOUTPUTに設定します
    //pinMode(wrb2sakura(WIFI_CTS), 1);
    //digitalWrite(wrb2sakura(WIFI_CTS), 1);
    //WiFiのシリアル3を設定
    //シリアル通信の初期化をします
    esp8266_serial_begin();
    int len;
    int ret;
    int cnt = 0;

    while (true) {
        //受信バッファを空にします
        while ((len = esp8266_serial_available()) > 0) {
            //RbSerial[0]->print(len);
            for (int i = 0; i < len; i++) {
                esp8266_serial_read();
            }
        }
        //ECHOオフコマンドを送信する
        esp8266_serial_println("ATE0");
        //OK 0d0a か ERROR 0d0aが来るまで wifi_data[]に読む
        ret = get_data(500);
        if (ret == 1) {
            //1の時は、WiFiが使用可能
            break;
        } else if (ret == 0) {
            //タイムアウトした場合は WiFiが使えないとする
            if (cnt >= 3)
                return 0;
        }
        //0,1で無いときは256バイト以上が返ってきている
        cnt++;
        if (cnt >= 3) {
            //3回ATE0を試みてダメだったら、あきらめる。
            return 0;
        }
    }
    return 1;
}

