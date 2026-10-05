#ifndef CMAIL_ENVELOPE_H
#define CMAIL_ENVELOPE_H

struct cmail_envelope {
    char* from;
    struct curl_slist *recipients;
};

struct cmail_envelope* cmail_envelope_create(const char* from, const char* recipient);
struct cmail_envelope* cmail_envelope_clone(const struct cmail_envelope* envelope);
void cmail_envelope_add_recipient(struct cmail_envelope* envelope, const char* recipient);
void cmail_envelope_clean(struct cmail_envelope* envelope);

#endif // CMAIL_ENVELOPE_H

