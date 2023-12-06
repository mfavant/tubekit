#pragma once
#include "connection/http_connection.h"

namespace tubekit
{
    namespace app
    {
        class http_app
        {
        public:
            static void process_connection(tubekit::connection::http_connection &m_http_connection);
            static int on_init();
            static void on_stop();
            static void on_tick();
        };
    }
}