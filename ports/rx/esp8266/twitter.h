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

#ifndef STWITTER_H_
#define STWITTER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OAUTH_SIGNATURE_METHOD  "HMAC-SHA1"
#define OAUTH_VERSION           "1.0"
#define TWITTER_API_UPDATE      "https://api.twitter.com/1.1/statuses/update.json"
#define TWITTER_API_UPDATE_STR  "api.twitter.com/1.1/statuses/update.json"
#define TWITTER_API_UPLOAD      "https://upload.twitter.com/1.1/media/upload.json"

typedef struct {
	char *_cons_key;
	char *_cons_sec;
	char *_accs_key;
	char *_accs_sec;
} twitter_t;

void twitter_api_init();
void twitter_api_deinit();
void twitter_api_set_keys(char *cons_key, char *cons_sec, char *accs_key, char *accs_sec);
void twitter_api_statuses_update(char *str, char *media_id_string);

#ifdef __cplusplus
}
#endif

#endif /* STWITTER_H_ */
