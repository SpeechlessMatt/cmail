#ifndef CMAIL_DISPATCH_H
#define CMAIL_DISPATCH_H

#include "error.h"
#include "obj.h"
#include "payload.h"
#include "envelope.h"

#define CMAIL_DISPATCH_SUCCESS 0
#define CMAIL_DISPATCH_ERROR 1

cmail_error_t cmail_send(
    const struct cmail_cred* cred, 
    const struct cmail_envelope* envelope, 
    const struct cmail_payload* payload
);

cmail_error_t cmail_send_with_option(
    const struct cmail_cred* cred, 
    const struct cmail_envelope* envelope, 
    const struct cmail_payload* payload,
    const struct cmail_options* option
);

#endif // CMAIL_DISPATCH_H
