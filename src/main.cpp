#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include "whitelist.h"
#include "cmd.h"
#include "errors.h"
#include "jsmn/jsmn.h"


#define EXAMPLE_RX_BUFFER_BYTES (1024)
struct payload
{
    unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + EXAMPLE_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
    string data1;
    size_t len;
} received_payload;


static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
        case LWS_CALLBACK_RECEIVE:
        {
            stringstream response;
            cmd_process((const char*)in, len, response);
            
            received_payload.data1  = response.str();
            received_payload.len = received_payload.data1.length();
            
            if (received_payload.len) {
                lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
            }
        }   break;
		case LWS_CALLBACK_HTTP:
        {
			lws_serve_http_file( wsi, "res/index.html", "text/html", NULL, 0 );
        }	break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
        {
            if (received_payload.len) {
                lws_write( wsi, (unsigned char *)received_payload.data1.c_str(), received_payload.len, LWS_WRITE_TEXT );
                //received_payload.data1 = "";
            }
        } break;
		default:
			break;
	}

	return 0;
}


static int callback_idevice_control( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
        case LWS_CALLBACK_CLOSED :
            lwsl_notice("CLOSED\n");
            break;
		case LWS_CALLBACK_RECEIVE:
            lwsl_notice("RECEIVED SOMETHING\n");
			memcpy( &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], in, len );
			received_payload.len = len;
			lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
            lwsl_notice("IS WRITABLE\n");
            if (received_payload.len) {
                lws_write( wsi, &received_payload.data[LWS_SEND_BUFFER_PRE_PADDING], received_payload.len, LWS_WRITE_TEXT );
            }
			break;

		default:
			break;
	}

	return 0;
}

enum protocols
{
	PROTOCOL_HTTP = 0,
	PROTOCOL_EXAMPLE,
	PROTOCOL_COUNT
};

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"idevice-control-protocol",
		callback_idevice_control,
		0,
		EXAMPLE_RX_BUFFER_BYTES,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int main( int argc, char *argv[] )
{
    int err = whitelist_init();
    if (err) {
        lwsl_err("Error reading whitelist configuration file. Error:%d\n", err);
        return 0;
    }
    
    struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	struct lws_context *context = lws_create_context( &info );

	while( 1 )
	{
		lws_service( context, 1000000 );
	}

	lws_context_destroy( context );
    
    whitelist_terminate();

	return 0;
}
