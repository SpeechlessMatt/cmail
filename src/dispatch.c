#include <stdlib.h>
#include <curl/curl.h>

#include "dispatch.h"
#include "error.h"

struct curl_data {
    const char *data;
    size_t length;
    size_t position;
};

static size_t cmail_curl_read_callback(
    char *buffer,
    size_t size,
    size_t nmemb,
    void *userdata
) {
    struct curl_data *data = userdata;

    size_t capacity = size * nmemb;
    size_t remaining = data->length - data->position;

    if (remaining > capacity) {
        remaining = capacity;
    }

    if (remaining > 0) {
        memcpy(
            buffer,
            data->data + data->position,
            remaining
        );

        data->position += remaining;
    }

    return remaining;
}

cmail_error_t cmail_send(
    const struct cmail_cred* cred, 
    const struct cmail_envelope* envelope, 
    const struct cmail_payload* payload
) {
    struct cmail_options option;
    cmail_options_init_defaults(&option);
    return cmail_send_with_option(cred, envelope, payload, &option);
}

cmail_error_t cmail_send_with_option(
    const struct cmail_cred* cred, 
    const struct cmail_envelope* envelope, 
    const struct cmail_payload* payload,
    const struct cmail_options* option
) {
    if (!cred || !envelope || !payload || !option) {
        return CMAIL_DISPATCH_ERROR;
    }

    char *message = NULL;
    size_t message_length = 0;

    cmail_error_t error = cmail_payload_build(payload, &message, &message_length);

    if (error != CMAIL_OK) {
        return error;
    }

    struct curl_data curl_data = {
        .data = message,
        .length = message_length,
        .position = 0
    };

    CURL *curl = curl_easy_init();
    if (!curl) {
        return CMAIL_DISPATCH_ERROR;
    }

    CURLcode res = CURLE_OK;

    curl_easy_setopt(curl, CURLOPT_URL, cred->smtp_url);
    if (cred->smtp_username) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, cred->smtp_username);
    }
    if (cred->smtp_password) {
        curl_easy_setopt(curl, CURLOPT_PASSWORD, cred->smtp_password);
    }

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, envelope->from);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, envelope->recipients);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)option->timeout_ms);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, cmail_curl_read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &curl_data);

    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    CURLcode result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    free(message);

    if (result != CURLE_OK) {
        return CMAIL_ERROR_IO;
    }

    return CMAIL_OK;
}
