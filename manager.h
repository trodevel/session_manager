/*

Authentication Manager.

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

// $Revision: 3573 $ $Date:: 2016-03-31 #$ $Author: serge $

#include <map>          // std::map
#include <set>          // std::set
#include <chrono>       // std::chrono::system_clock::time_point
#include <mutex>        // std::mutex


namespace session_manager
{

class IAuthenticator
{
public:
    virtual ~IAuthenticator() {}

    virtual bool is_authenticated( const std::string & user_id, const std::string & password ) const    = 0;
};

class Manager
{
public:

    struct Config
    {
        uint16_t    expiration_time;    // in minutes
        uint16_t    max_sessions_per_user;
        bool        prolong_expiration;
    };

public:
    Manager();

    bool init( IAuthenticator * auth, const Config & config );

    bool authenticate( const std::string & user_id, const std::string & password, std::string & session_id, std::string & error );
    bool close_session( const std::string & session_id, std::string & error );

    bool is_authenticated( const std::string & session_id );

private:

    struct Session
    {
        std::chrono::system_clock::time_point started;
        std::chrono::system_clock::time_point expire;

        bool is_expired() const;
    };

    typedef std::map<std::string,Session>       MapSessionIdToSession;
    typedef std::map<std::string,std::string>   MapSessionIdToUser;

    typedef std::map<std::string,std::set<std::string>>    MapUserToSessionList;

private:

    void remove_expired();

    void init_new_session( Session & sess );

    void add_new_session( MapUserToSessionList::mapped_type & sess_set, const std::string & user_id, std::string & session_id );

    bool remove_session( const std::string & session_id, std::string & error );

private:
    mutable std::mutex      mutex_;


    IAuthenticator          * auth_;

    Config                  config_;

    MapSessionIdToSession   map_sessions_;
    MapUserToSessionList    map_user_to_sessions_;
    MapSessionIdToUser      map_session_to_user_;
};

}

