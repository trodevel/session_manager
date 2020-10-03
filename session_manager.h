/*

Session Manager.

Copyright (C) 2016 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 13920 $ $Date:: 2020-10-03 #$ $Author: serge $

#ifndef SESSION_MANAGER__MANAGER_H
#define SESSION_MANAGER__MANAGER_H

#include <map>          // std::map
#include <set>          // std::set
#include <chrono>       // std::chrono::system_clock::time_point
#include <mutex>        // std::mutex

#include "config.h"     // Config
#include "types.h"      // user_id_t

namespace session_manager
{

class IAuthenticator;

class SessionManager
{
public:

    struct SessionInfo
    {
        user_id_t                               user_id;
        std::chrono::system_clock::time_point   start_time;
        std::chrono::system_clock::time_point   expiration_time;
    };

public:
    SessionManager();

    void init( IAuthenticator * auth, const Config & config );

    bool authenticate( user_id_t user_id, const std::string & password, std::string & session_id, std::string & error );
    bool close_session( const std::string & session_id, std::string & error );

    bool is_authenticated( const std::string & session_id );
    bool get_user_id( user_id_t * user_id, const std::string & session_id );
    bool get_session_info( SessionInfo * session_info, const std::string & session_id );

private:

    struct Session
    {
        std::chrono::system_clock::time_point started;
        std::chrono::system_clock::time_point expire;

        bool is_expired() const;
    };

    typedef std::map<std::string,Session>       MapSessionIdToSession;
    typedef std::map<std::string,user_id_t>     MapSessionIdToUser;

    typedef std::map<user_id_t,std::set<std::string>>    MapUserToSessionList;

private:

    void remove_expired();

    void init_new_session( Session & sess );
    void postpone_expiration( Session & sess );

    void add_new_session( MapUserToSessionList::mapped_type & sess_set, user_id_t user_id, std::string & session_id );

    bool remove_session( const std::string & session_id, std::string & error );
    bool get_associated_session( SessionInfo * session_info, const std::string & session_id, bool is_user_request );

private:
    mutable std::mutex      mutex_;


    IAuthenticator          * auth_;

    Config                  config_;

    MapSessionIdToSession   map_sessions_;
    MapUserToSessionList    map_user_to_sessions_;
    MapSessionIdToUser      map_session_to_user_;
};

}

#endif // SESSION_MANAGER__MANAGER_H
