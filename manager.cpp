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

// $Revision: 6192 $ $Date:: 2017-03-22 #$ $Author: serge $

#include "manager.h"        // self

#include <cassert>          // std::assert

#include "gen_uuid.h"       // gen_uuid
#include "i_authenticator.h"            // IAuthenticator

#include "../utils/mutex_helper.h"      // MUTEX_SCOPE_LOCK
#include "../utils/dummy_logger.h"      // dummy_log

#define MODULENAME      "Manager"


namespace session_manager
{

Manager::Manager():
        auth_( nullptr )
{
}

bool Manager::init( IAuthenticator * auth, const Config & config )
{
    if( auth == nullptr || config.expiration_time == 0 || config.max_sessions_per_user == 0 )
        return false;

    auth_   = auth;
    config_ = config;

    dummy_log_info( MODULENAME, "init: OK" );

    return true;
}

bool Manager::authenticate( user_id_t user_id, const std::string & password, std::string & session_id, std::string & error )
{
    dummy_log_debug( MODULENAME, "authenticate: user %u, password ...", user_id );

    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    if( auth_->is_authenticated( user_id, password ) == false )
    {
        error = "authentication failed";
        return false;
    }

    auto it = map_user_to_sessions_.find( user_id );

    if( it == map_user_to_sessions_.end() )
    {
        // user has no sessions yet

        MapUserToSessionList::mapped_type sess_set;

        add_new_session( sess_set, user_id, session_id );

        {
            bool _b = map_user_to_sessions_.insert( MapUserToSessionList::value_type( user_id, sess_set )).second;

            assert( _b );
        }
    }
    else
    {
        if( it->second.size() == config_.max_sessions_per_user )
        {
            error = "max number of sessions was reached (" + std::to_string( config_.max_sessions_per_user ) + ")";
            return false;
        }
        else
        {
            auto & sess_set = it->second;

            add_new_session( sess_set, user_id, session_id );
        }
    }

    dummy_log_debug( MODULENAME, "authenticate: OK: user %u, session_id %s", user_id, session_id.c_str() );

    return true;
}

bool Manager::close_session( const std::string & session_id, std::string & error )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    return remove_session( session_id, error );
}

bool Manager::remove_session( const std::string & session_id, std::string & error )
{
    dummy_log_debug( MODULENAME, "remove_session: session %s", session_id.c_str() );

    {
        // remove session from session map
        auto it = map_sessions_.find( session_id );

        if( it == map_sessions_.end() )
        {
            error = "invalid session id or session has already expired";
            return false;
        }

        map_sessions_.erase( it );
    }

    user_id_t user_id;
    {
        // remove session from session-to-user map
        auto it = map_session_to_user_.find( session_id );

        assert( it != map_session_to_user_.end() );

        user_id = it->second;

        map_session_to_user_.erase( it );
    }

    {
        // remove session from user-to-session map
        auto it = map_user_to_sessions_.find( user_id );

        assert( it != map_user_to_sessions_.end() );

        auto _num_del = it->second.erase( session_id );

        assert( _num_del > 0 );
    }

    return true;
}

void Manager::remove_expired()
{
    std::string error;

    std::size_t num_expired = 0;

    for( auto & v : map_sessions_ )
    {
        if( v.second.is_expired() )
        {
            num_expired++;
            remove_session( v.first, error );
        }
    }

    dummy_log_debug( MODULENAME, "remove_expired: number of expired sessions = %u", num_expired );
}

void Manager::init_new_session( Session & sess )
{
    sess.started    = std::chrono::system_clock::now();
    sess.expire     = sess.started + std::chrono::minutes( config_.expiration_time );
}

void Manager::postpone_expiration( Session & sess )
{
    sess.expire     = std::chrono::system_clock::now() + std::chrono::minutes( config_.expiration_time );
}

void Manager::add_new_session( MapUserToSessionList::mapped_type & sess_set, user_id_t user_id, std::string & session_id )
{
    Session sess;

    init_new_session( sess );

    session_id = gen_uuid();

    dummy_log_debug( MODULENAME, "add_new_session: session %s, user %u", session_id.c_str(), user_id );

    sess_set.insert( session_id );

    {
        bool _b = map_sessions_.insert( MapSessionIdToSession::value_type( session_id, sess )).second;

        assert( _b );
    }

    {
        bool _b = map_session_to_user_.insert( MapSessionIdToUser::value_type( session_id, user_id )).second;

        assert( _b );
    }

    dummy_log_debug( MODULENAME, "add_new_session: total number of sessions = %u", map_sessions_.size() );
}

bool Manager::is_authenticated( const std::string & session_id, user_id_t & user_id )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    if( map_sessions_.count( session_id ) == 0 )
    {
        dummy_log_debug( MODULENAME, "is_authenticated: NO: unknown session_id %s", session_id.c_str() );
        return false;
    }

    auto it = map_session_to_user_.find( session_id );

    assert( it != map_session_to_user_.end() );

    user_id = it->second;

    if( config_.postpone_expiration )
    {
        postpone_expiration( map_sessions_[ session_id ] );
    }

    dummy_log_debug( MODULENAME, "is_authenticated: OK: session_id %s", session_id.c_str() );

    return true;
}

bool Manager::is_authenticated( const std::string & session_id )
{
    user_id_t dummy;

    return is_authenticated( session_id, dummy );
}

Manager::user_id_t Manager::get_user_id( const std::string & session_id )
{
    user_id_t res;

    return is_authenticated( session_id, res ) ? res : 0;
}

bool Manager::Session::is_expired() const
{
    auto now = std::chrono::system_clock::now();

    return ( now >= expire ) ? true : false;
}

}

