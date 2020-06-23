#include "pragma.h"
#include "json_compat.h"
#include "purple_compat.h"
#include "libsignald.h"
#include "message.h"
#include "direct.h"

#pragma GCC diagnostic pop

void
signald_process_direct_message(SignaldAccount *sa, SignaldMessage *msg)
{
    PurpleIMConversation *imconv = purple_conversations_find_im_with_account(msg->conversation_name, sa->account);

    if (imconv == NULL) {
        imconv = purple_im_conversation_new(sa->account, msg->conversation_name);
    }

    PurpleMessageFlags flags = PURPLE_MESSAGE_RECV;
    GString *content = NULL;
    gboolean has_attachment = FALSE;

    if (! signald_format_message(sa, msg, &content, &has_attachment)) {
        return;
    }

    if (has_attachment) {
        flags |= PURPLE_MESSAGE_IMAGES;
    }

    if (msg->is_sync_message) {
        flags |= PURPLE_MESSAGE_SEND | PURPLE_MESSAGE_REMOTE_SEND | PURPLE_MESSAGE_DELAYED;

        purple_conv_im_write(imconv, msg->conversation_name, content->str, flags, msg->timestamp);
    } else {
        purple_serv_got_im(sa->pc, msg->conversation_name, content->str, flags, msg->timestamp);
    }

    g_string_free(content, TRUE);
}

int
signald_send_im(PurpleConnection *pc,
#if PURPLE_VERSION_CHECK(3, 0, 0)
                PurpleMessage *msg)
{
    const gchar *who = purple_message_get_recipient(msg);
    const gchar *message = purple_message_get_contents(msg);
#else
                const gchar *who, const gchar *message, PurpleMessageFlags flags)
{
#endif
    purple_debug_info(SIGNALD_PLUGIN_ID, "signald_send_im: flags: %x msg:%s\n", flags, message);

    if (purple_strequal(who, SIGNALD_UNKNOWN_SOURCE_NUMBER)) {
        return 0;
    }

    SignaldAccount *sa = purple_connection_get_protocol_data(pc);

    return signald_send_message(sa, SIGNALD_MESSAGE_TYPE_DIRECT, (char *)who, message);
}
