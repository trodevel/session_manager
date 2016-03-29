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

// $Revision: 3564 $ $Date:: 2016-03-29 #$ $Author: serge $

#include "manager.h"        // self


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

    return true;
}


bool Manager::authenticate( const std::string & user_id, const std::string & password, std::string & session_id, std::string & error )
{
    if( auth_->is_authenticated( user_id, password ) == false )
    {
        error = "authentication failed";
        return false;
    }

    return true;
}

bool Manager::close_session( const std::string & session_id, std::string & error )
{

}

void Manager::remove_expired()
{
}

bool Manager::is_authenticated( const std::string & session_id )
{
    remove_expired();

    return ( map_sessions_.count( session_id ) > 0 ) ? true : false;
}

}

