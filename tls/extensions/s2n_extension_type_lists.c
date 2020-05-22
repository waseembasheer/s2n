/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <s2n.h>

#include "tls/extensions/s2n_extension_type_lists.h"
#include "tls/s2n_connection.h"

#include "tls/extensions/s2n_cookie.h"
#include "tls/extensions/s2n_client_supported_versions.h"
#include "tls/extensions/s2n_client_signature_algorithms.h"
#include "tls/extensions/s2n_client_max_frag_len.h"
#include "tls/extensions/s2n_client_session_ticket.h"
#include "tls/extensions/s2n_client_server_name.h"
#include "tls/extensions/s2n_client_alpn.h"
#include "tls/extensions/s2n_client_status_request.h"
#include "tls/extensions/s2n_client_key_share.h"
#include "tls/extensions/s2n_client_sct_list.h"
#include "tls/extensions/s2n_client_supported_groups.h"
#include "tls/extensions/s2n_client_pq_kem.h"
#include "tls/extensions/s2n_client_renegotiation_info.h"
#include "tls/extensions/s2n_ec_point_format.h"
#include "tls/extensions/s2n_server_certificate_status.h"
#include "tls/extensions/s2n_server_renegotiation_info.h"
#include "tls/extensions/s2n_server_alpn.h"
#include "tls/extensions/s2n_server_status_request.h"
#include "tls/extensions/s2n_server_sct_list.h"
#include "tls/extensions/s2n_server_max_fragment_length.h"
#include "tls/extensions/s2n_server_session_ticket.h"
#include "tls/extensions/s2n_server_server_name.h"
#include "tls/extensions/s2n_server_signature_algorithms.h"
#include "tls/extensions/s2n_server_supported_versions.h"
#include "tls/extensions/s2n_server_key_share.h"

static const s2n_extension_type *const client_hello_extensions[] = {
        &s2n_client_supported_versions_extension,
        &s2n_client_key_share_extension,
        &s2n_client_signature_algorithms_extension,
        &s2n_client_server_name_extension,
        &s2n_client_alpn_extension,
        &s2n_client_status_request_extension,
        &s2n_client_sct_list_extension,
        &s2n_client_max_frag_len_extension,
        &s2n_client_session_ticket_extension,
        &s2n_client_supported_groups_extension,
        &s2n_client_ec_point_format_extension,
        &s2n_client_pq_kem_extension,
        &s2n_client_renegotiation_info_extension,
};

static const s2n_extension_type *const tls12_server_hello_extensions[] = {
        &s2n_server_supported_versions_extension,
        &s2n_server_server_name_extension,
        &s2n_server_ec_point_format_extension,
        &s2n_server_renegotiation_info_extension,
        &s2n_server_alpn_extension,
        &s2n_server_status_request_extension,
        &s2n_server_sct_list_extension,
        &s2n_server_max_fragment_length_extension,
        &s2n_server_session_ticket_extension,
};

static const s2n_extension_type *const tls13_server_hello_extensions[] = {
        &s2n_server_supported_versions_extension,
        &s2n_server_key_share_extension,
        &s2n_server_cookie_extension,
};

static const s2n_extension_type *const encrypted_extensions[] = {
        &s2n_server_server_name_extension,
        &s2n_server_max_fragment_length_extension,
        &s2n_server_alpn_extension,
};

static const s2n_extension_type *const cert_req_extensions[] = {
        &s2n_server_signature_algorithms_extension,
};

static const s2n_extension_type *const certificate_extensions[] = {
        &s2n_tls13_server_status_request_extension,
        &s2n_server_sct_list_extension,
};

#define S2N_EXTENSION_LIST(list) { .extension_types = (list), .count = s2n_array_len(list) }
#define S2N_EMPTY_EXTENSION_LIST { .extension_types = NULL, .count = 0 }

static struct {
    s2n_extension_type_list default_list;
    s2n_extension_type_list tls13_list; /* TLS1.3 moved some extensions to different extension lists */
} extension_lists[] = {
        [S2N_EXTENSION_LIST_CLIENT_HELLO] = {
                .default_list = S2N_EXTENSION_LIST(client_hello_extensions),
                .tls13_list = S2N_EXTENSION_LIST(client_hello_extensions),
        },
        [S2N_EXTENSION_LIST_SERVER_HELLO] = {
                .default_list = S2N_EXTENSION_LIST(tls12_server_hello_extensions),
                .tls13_list = S2N_EXTENSION_LIST(tls13_server_hello_extensions),
        },
        [S2N_EXTENSION_LIST_ENCRYPTED_EXTENSIONS] = {
                .default_list = S2N_EMPTY_EXTENSION_LIST,
                .tls13_list = S2N_EXTENSION_LIST(encrypted_extensions),
        },
        [S2N_EXTENSION_LIST_CERT_REQ] = {
                .default_list = S2N_EMPTY_EXTENSION_LIST,
                .tls13_list = S2N_EXTENSION_LIST(cert_req_extensions),
        },
        [S2N_EXTENSION_LIST_CERTIFICATE] = {
                .default_list = S2N_EMPTY_EXTENSION_LIST,
                .tls13_list = S2N_EXTENSION_LIST(certificate_extensions),
        },
};

int s2n_extension_type_list_get(s2n_extension_list_id list_type, const struct s2n_connection *conn,
        s2n_extension_type_list **extension_list)
{
    notnull_check(extension_list);
    notnull_check(conn);
    lt_check(list_type, s2n_array_len(extension_lists));

    if (s2n_connection_get_protocol_version(conn) >= S2N_TLS13) {
        *extension_list = &extension_lists[list_type].tls13_list;
    } else {
        *extension_list = &extension_lists[list_type].default_list;
    }

    return S2N_SUCCESS;
}