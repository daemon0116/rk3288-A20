/*
 * Win32DebugAppender.hh
 *
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef _LOG4CPP_ANDROIDAPPENDER_HH
#define _LOG4CPP_ANDROIDAPPENDER_HH


#include <string>
#include "log4cpp/Export.hh"
#include "log4cpp/LayoutAppender.hh"
#define ERROR  300
#define WARN   400
#define NOTICE 500
#define INFO   600
#define DEBUG  700


namespace log4cpp {

    /**
     * AndroidAppender simply sends the log message to the default system
     * Android. 
     */
    class LOG4CPP_EXPORT AndroidAppender : public LayoutAppender {
        public:
        /**
         * Constructor.
         * @param name Name used by the base classes only.
         */
        AndroidAppender(const std::string& name);
        /**
         * Destructor.
         */
        virtual ~AndroidAppender();
        
        /**
         * Close method.  This is called by the framework, but there is nothing
         * to do for the OutputDebugString API, so it simply returns.
         */
        virtual void close();

        protected:
        /**
         * Method that does the actual work.  In this case, it simply sets up the layout
         * and calls the OutputDebugString API.
         * @param event Event for which we are logging.
         */
        virtual void _append(const LoggingEvent& event);


    };
}
#endif // _LOG4CPP_ANDROIDAPPENDER_HH
